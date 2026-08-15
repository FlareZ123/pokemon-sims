#ifndef SIM_TRACE_SETUP_LIFECYCLE_INCLUDED
#define SIM_TRACE_SETUP_LIFECYCLE_INCLUDED

  [[nodiscard]] bool setup_recipe_contains(const Card card) const {
    return total_copies(card) > 0;
  }

  [[nodiscard]] bool setup_recipe_has_exactly(const Card card, const int copies) const {
    return total_copies(card) == copies;
  }

  [[nodiscard]] std::string current_deck_id() const {
    if (setup_recipe_contains(Card::DoubleDragonEnergy)) {
      return "regidrago-dde-model";  // Modeling recipe: https://github.com/FlareZ123/pokemon-sims/issues/2238 ; DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    }
    if (setup_recipe_contains(Card::CrobatV)) {
      return "modeling-crobat-variant";
    }
    if (setup_recipe_contains(Card::SecretBox) && setup_recipe_contains(Card::Pineco)) {
      return "regidrago-pineco";
    }
    if (setup_recipe_has_exactly(Card::RegidragoV, 4) &&
        setup_recipe_has_exactly(Card::BrilliantBlender, 1)) {
      return "regidrago-shell";
    }
    return "custom";
  }

  struct SetupLifecycleConfig {
    inline static constexpr int kOpeningHandSize = 7;
    inline static constexpr int kStartingPrizeCount = 6;

    [[nodiscard]] static const char* dci_label(const DciProfile profile) {
      // DCI/JIT policy vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      switch (profile) {
        case DciProfile::StrictJit: return "strict-JIT";
        case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
        case DciProfile::NoDiscardControl: return "no-discard-control";
      }
      return "unknown";
    }

    [[nodiscard]] static const char* lock_label(const LockMode mode) {
      // Scenario lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
      switch (mode) {
        case LockMode::None: return "none";
        case LockMode::TurnTwoItem: return "turn-two Item lock";
        case LockMode::FullItem: return "full Item lock";
        case LockMode::FullRuleBoxAbility: return "full Rule Box Ability lock";
        case LockMode::FullCombined: return "full Item + Rule Box Ability lock";
        case LockMode::FullSupporter: return "full Supporter lock";
      }
      return "unknown";
    }
  };

  [[nodiscard]] std::string dci_name() const {
    return SetupLifecycleConfig::dci_label(scenario_.dci);
  }

  [[nodiscard]] std::string lock_name() const {
    return SetupLifecycleConfig::lock_label(scenario_.locks);
  }

  void move_top_cards_from_deck(std::vector<Card>& destination, int count) {
    // Setup draws and Prize placement both move cards from the already-shuffled deck
    // without introducing an extra shuffle between those steps:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    for (int card = 0; card < count; ++card) {
      destination.push_back(state_.deck.back());
      state_.deck.pop_back();
    }
  }

  void add_recipe_cards_to_deck() {
    for (const auto& [card, copies] : recipe_) {
      for (int copy = 0; copy < copies; ++copy) state_.deck.push_back(card);
    }
  }

  void reset_setup_knowledge() {
    deck_seen_ = false;
    prizes_revealed_ = false;
  }

  bool opening_hand_has_basic() const {
    // A valid opening hand must contain at least one Basic Pokémon before setup can continue:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    return std::any_of(state_.hand.begin(), state_.hand.end(), is_basic);
  }

  void return_mulligan_hand_to_deck() {
    // A mulligan returns the seven-card hand to the deck before the deck is shuffled again:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
  }

  void draw_opening_hand_with_mulligans() {
    while (true) {
      state_.hand.clear();
      move_top_cards_from_deck(state_.hand, SetupLifecycleConfig::kOpeningHandSize);
      if (opening_hand_has_basic()) return;
      return_mulligan_hand_to_deck();
      shuffle(state_.deck);
      ++outcome_.mulligans;
    }
  }

  void deal_prizes_after_opening_setup() {
    // Place six Prize cards from the already-shuffled remaining deck after the
    // opening Active and optional Bench are selected. The setup procedure does
    // not add another shuffle at this point: https://www.pokemon.com/us/pokemon-tcg/rules
    move_top_cards_from_deck(state_.prizes, SetupLifecycleConfig::kStartingPrizeCount);
  }

  void emit_setup_trace() {
    if (trace_ == nullptr || !trace_->enabled) return;
    trace_->note("===== SIMULATE THIS =====");
    trace_->note("Deck: " + current_deck_id());
    trace_->note("Scenario: " + scenario_.label + " | DCI: " + dci_name() + " | Locks: " + lock_name());
    trace_->note("Mulligans: " + std::to_string(outcome_.mulligans));
    trace_->note("Opening player-known state: " + state_line());
    trace_->note("DEBUG ONLY: prizes=" + join_cards(state_.prizes) + ". Policy may use them only after a legal deck or Prize inspection.");
    trace_->note("Policy rule: strict and matchup-flex JIT require a payload to enter discard in the same player turn as the ready-state check.");
  }

  void setup() {
    reset_setup_knowledge();
    add_recipe_cards_to_deck();
    shuffle(state_.deck);
    draw_opening_hand_with_mulligans();

    choose_opening_active();
    outcome_.started_tapu = state_.active && state_.active->card == Card::TapuLeleGX;
    choose_opening_bench();
    deal_prizes_after_opening_setup();
    emit_setup_trace();
  }

#endif  // SIM_TRACE_SETUP_LIFECYCLE_INCLUDED
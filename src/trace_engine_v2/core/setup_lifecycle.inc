#ifndef SIM_TRACE_SETUP_LIFECYCLE_INCLUDED
#define SIM_TRACE_SETUP_LIFECYCLE_INCLUDED

  bool simulation_deck_contains(Card card) const {
    return total_copies(card) > 0;
  }

  bool is_regidrago_shell_recipe() const {
    return total_copies(Card::RegidragoV) == 4 &&
           total_copies(Card::BrilliantBlender) == 1;
  }

  std::string current_deck_id() const {
    if (simulation_deck_contains(Card::DoubleDragonEnergy)) {
      return "regidrago-dde-model";  // Modeling recipe: https://github.com/FlareZ123/pokemon-sims/issues/2238 ; DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    }
    if (simulation_deck_contains(Card::CrobatV)) {
      return "modeling-crobat-variant";
    }
    if (simulation_deck_contains(Card::SecretBox) &&
        simulation_deck_contains(Card::Pineco)) {
      return "regidrago-pineco";
    }
    if (is_regidrago_shell_recipe()) {
      return "regidrago-shell";
    }
    return "custom";
  }

  static const char* dci_label(DciProfile profile) {
    // DCI/JIT policy vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    switch (profile) {
      case DciProfile::StrictJit: return "strict-JIT";
      case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
      case DciProfile::NoDiscardControl: return "no-discard-control";
    }
    return "unknown";
  }

  static const char* lock_label(LockMode mode) {
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

  std::string dci_name() const {
    return dci_label(scenario_.dci);
  }

  std::string lock_name() const {
    return lock_label(scenario_.locks);
  }

  // Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  inline static constexpr int kOpeningHandSize = 7;
  inline static constexpr int kStartingPrizeCount = 6;

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

  void draw_opening_hand_with_mulligans() {
    while (true) {
      state_.hand.clear();
      move_top_cards_from_deck(state_.hand, kOpeningHandSize);
      if (std::any_of(state_.hand.begin(), state_.hand.end(), is_basic)) return;
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      state_.hand.clear();
      shuffle(state_.deck);
      ++outcome_.mulligans;
    }
  }

  void deal_prizes_after_opening_setup() {
    // Place six Prize cards from the already-shuffled remaining deck after the
    // opening Active and optional Bench are selected. The setup procedure does
    // not add another shuffle at this point: https://www.pokemon.com/us/pokemon-tcg/rules
    move_top_cards_from_deck(state_.prizes, kStartingPrizeCount);
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

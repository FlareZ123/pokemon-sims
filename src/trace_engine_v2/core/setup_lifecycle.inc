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

  void prepare_setup_deck() {
    add_recipe_cards_to_deck();
    shuffle(state_.deck);
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
    prepare_setup_deck();
    draw_opening_hand_with_mulligans();

    choose_opening_active();
    outcome_.started_tapu = state_.active && state_.active->card == Card::TapuLeleGX;
    choose_opening_bench();
    deal_prizes_after_opening_setup();
    emit_setup_trace();
  }
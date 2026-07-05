    out << " | Discard=" << join_cards(state_.discard);
    return out.str();
  }

  void setup() {
    deck_seen_ = false;
    prizes_revealed_ = false;
    for (const auto& [card, copies] : recipe_) {
      for (int copy = 0; copy < copies; ++copy) state_.deck.push_back(card);
    }
    shuffle(state_.deck);
    while (true) {
      state_.hand.clear();
      for (int card = 0; card < 7; ++card) {
        const Card drawn = state_.deck.back();
        state_.deck.pop_back();
        state_.hand.push_back(drawn);
      }
      if (std::any_of(state_.hand.begin(), state_.hand.end(), is_basic)) break;
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      state_.hand.clear();
      shuffle(state_.deck);
      ++outcome_.mulligans;
    }

    choose_opening_active();
    choose_opening_bench();
    shuffle(state_.deck);
    for (int prize = 0; prize < 6; ++prize) {
      state_.prizes.push_back(state_.deck.back());
      state_.deck.pop_back();
    }

    if (trace_ != nullptr && trace_->enabled) {
      trace_->note("===== SIMULATE THIS =====");
      trace_->note("Scenario: " + scenario_.label + " | DCI: " + dci_name() + " | Locks: " + lock_name());
      trace_->note("Mulligans: " + std::to_string(outcome_.mulligans));
      trace_->note("Opening player-known state: " + state_line());
      trace_->note("DEBUG ONLY: prizes=" + join_cards(state_.prizes) + ". Policy may use them only after a legal deck or Prize inspection.");
      trace_->note("Policy rule: strict and matchup-flex JIT require a payload to enter discard in the same player turn as the ready-state check.");
    }
  }

  std::string dci_name() const {
    switch (scenario_.dci) {
      case DciProfile::StrictJit: return "strict-JIT";
      case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
      case DciProfile::NoDiscardControl: return "no-discard-control";
    }
    return "unknown";
  }

  std::string lock_name() const {
    switch (scenario_.locks) {
      case LockMode::None: return "none";
      case LockMode::TurnTwoItem: return "turn-two Item lock";
      case LockMode::FullItem: return "full Item lock";
      case LockMode::FullRuleBoxAbility: return "full Rule Box Ability lock";
      case LockMode::FullCombined: return "full Item + Rule Box Ability lock";
    }
    return "unknown";
  }

  void choose_opening_active() {
    for (const Card candidate : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::MawileGX, Card::DialgaGX, Card::TapuLeleGX}) {
      if (remove_one(state_.hand, candidate)) {
        state_.active = Pokemon{candidate, 0};
        outcome_.started_regi = candidate == Card::RegidragoV;
        return;
      }
    }
    throw std::runtime_error("opening hand has no Basic after mulligan validation");
  }

  void choose_opening_bench() {
    if (bench_space() > 0 && hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) < 2) {
      remove_one(state_.hand, Card::RegidragoV);
      state_.bench.push_back(Pokemon{Card::RegidragoV, 0});
    }
    if (bench_space() > 0 && hand_count(Card::LatiasEx) > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && !active_is_regi_v()) {
      remove_one(state_.hand, Card::LatiasEx);
      state_.bench.push_back(Pokemon{Card::LatiasEx, 0});
    }
  }

  bool active_is_regi_v() const { return state_.active && state_.active->card == Card::RegidragoV; }

  bool move_deck_to_hand(const Card card) {
    record_deck_search_knowledge("search effect");
    if (!remove_one(state_.deck, card)) return false;
    state_.hand.push_back(card);
    return true;
  }

  bool move_deck_to_discard(const Card card) {
    record_deck_search_knowledge("search effect");
    if (!remove_one(state_.deck, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    return true;
  }

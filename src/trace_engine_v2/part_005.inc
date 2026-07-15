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
    // Place six Prize cards from the already-shuffled remaining deck after the
    // opening Active and optional Bench are selected. The setup procedure does
    // not add another shuffle at this point: https://www.pokemon.com/us/pokemon-tcg/rules
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
    const bool tapu_oricorio_are_only_opening_basics =
        hand_count(Card::TapuLeleGX) > 0 && hand_count(Card::Oricorio) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::DialgaGX) == 0;
    const bool rule_box_ability_lock =
        scenario_.locks == LockMode::FullRuleBoxAbility ||
        scenario_.locks == LockMode::FullCombined;
    const bool crispin_route_already_held = hand_count(Card::Crispin) > 0;

    // Setup permits either Basic to become Active. Preserve Oricorio for Vital Dance
    // when Wonder Tag is suppressed, or when Crispin already supplies the Supporter
    // route and the Energy-compression Ability is the scarce on-play connector:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    if (tapu_oricorio_are_only_opening_basics &&
        (rule_box_ability_lock || crispin_route_already_held) &&
        remove_one(state_.hand, Card::TapuLeleGX)) {
      state_.active = Pokemon{Card::TapuLeleGX, 0};
      outcome_.started_regi = false;
      return;
    }

    const bool tapu_dialga_are_only_opening_basics =
        hand_count(Card::TapuLeleGX) > 0 && hand_count(Card::DialgaGX) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::Oricorio) == 0;
    const int payloads_in_hand = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_payload));
    const bool dialga_is_unique_in_hand_payload =
        hand_count(Card::DialgaGX) == 1 && payloads_in_hand == 1;

    // Dialga-GX is a modeled Apex Dragon payload. Under strict current-turn DCI,
    // preserve the unique in-hand payload when Crispin already covers the Supporter
    // connector and Tapu Lele-GX can legally start instead:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    if (tapu_dialga_are_only_opening_basics && strict_payload_timing() &&
        dialga_is_unique_in_hand_payload && crispin_route_already_held &&
        remove_one(state_.hand, Card::TapuLeleGX)) {
      state_.active = Pokemon{Card::TapuLeleGX, 0};
      outcome_.started_regi = false;
      return;
    }

    const bool oricorio_dialga_are_only_opening_basics =
        hand_count(Card::Oricorio) > 0 && hand_count(Card::DialgaGX) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::TapuLeleGX) == 0;
    const bool separate_payload_remains_in_hand =
        std::any_of(state_.hand.begin(), state_.hand.end(), [](const Card card) {
          return card != Card::DialgaGX && is_payload(card);
        });
    const bool vital_dance_advances_opening_energy =
        ability_available_for_pokemon(Card::Oricorio) &&
        (hand_count(Card::Grass) < 2 || hand_count(Card::Fire) < 1);

    // When another modeled payload remains, Dialga-GX can become the opening Active
    // without consuming the strict-JIT payload axis. Preserve Oricorio in hand so
    // playing it onto the Bench can use Vital Dance to supply a missing GGF component:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (oricorio_dialga_are_only_opening_basics && strict_payload_timing() &&
        separate_payload_remains_in_hand && vital_dance_advances_opening_energy &&
        remove_one(state_.hand, Card::DialgaGX)) {
      state_.active = Pokemon{Card::DialgaGX, 0};
      outcome_.started_regi = false;
      return;
    }

    // Oricorio's Vital Dance triggers only when it is played from hand onto the
    // Bench during the turn. When Mawile-GX is the other available opening Basic,
    // start Mawile-GX so Oricorio remains in hand for that legal setup trigger:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sm11-141
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/RULES_AND_INTERACTIONS.md#deliberate-bounds
    for (const Card candidate : {Card::RegidragoV, Card::LatiasEx, Card::MawileGX, Card::Oricorio, Card::DialgaGX, Card::TapuLeleGX}) {
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
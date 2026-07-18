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
    const bool grass_energy_already_held = hand_count(Card::Grass) > 0;
    const bool fire_energy_already_held = hand_count(Card::Fire) > 0;
    const bool manual_energy_already_held =
        grass_energy_already_held || fire_energy_already_held;
    const bool both_manual_energy_types_held =
        grass_energy_already_held && fire_energy_already_held;
    const bool duplicate_crispin_competes_for_missing_energy_type =
        hand_count(Card::Crispin) > 1 && manual_energy_already_held &&
        !both_manual_energy_types_held;
    const bool communication_search_already_held =
        pokemon_communication_opening_route_has_safe_exchange();
    const bool direct_pokemon_search_already_held =
        hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::QuickBall) > 0 ||
        hand_count(Card::UltraBall) > 0 || communication_search_already_held;
    const bool wonder_tag_retains_higher_marginal_value =
        manual_energy_already_held && hand_count(Card::Arven) > 0 &&
        direct_pokemon_search_already_held &&
        !duplicate_crispin_competes_for_missing_energy_type;
    const bool crispin_makes_vital_dance_scarce =
        crispin_route_already_held && !wonder_tag_retains_higher_marginal_value;

    // Setup permits either Basic to become Active. Preserve Oricorio for Vital Dance
    // when Wonder Tag is suppressed, or when Crispin already supplies the Supporter
    // route and the Energy-compression Ability remains the scarce on-play connector.
    // Energy, Arven, and direct Pokemon search normally preserve Wonder Tag, except
    // when duplicate Crispin and Arven compete for the Supporter slot while one GGF
    // Energy type is still missing. Vital Dance then supplies the missing type without
    // consuming that Supporter play:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/654
    // https://github.com/FlareZ123/pokemon-sims/issues/678
    if (tapu_oricorio_are_only_opening_basics &&
        (rule_box_ability_lock || crispin_makes_vital_dance_scarce) &&
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
    const bool held_turo_fss_recovery_graph =
        hand_count(Card::ProfessorTuro) > 0 && hand_count(Card::ForestSealStone) > 0;
    const bool payable_quick_ball_regi_route =
        hand_count(Card::QuickBall) > 0 &&
        choose_discard(false, true, true, Card::QuickBall).has_value();
    const bool payable_mysterious_treasure_regi_route =
        hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(false, true, true, Card::MysteriousTreasure).has_value();
    const bool held_redundant_one_discard_regi_graph =
        !item_locked() && might_be_unseen(Card::RegidragoV) &&
        (payable_quick_ball_regi_route || payable_mysterious_treasure_regi_route) &&
        payloads_in_hand > 1 && hand_count(Card::Crispin) == 1 &&
        hand_count(Card::BrilliantBlender) == 0;
    const bool dialga_has_route_preservation_value =
        hand_count(Card::DialgaGX) == 1 &&
        (dialga_is_unique_in_hand_payload || held_turo_fss_recovery_graph ||
         held_redundant_one_discard_regi_graph);

    // Quick Ball and Mysterious Treasure expose Regidrago V only when the same
    // centralized DCI selector used by their real play paths admits the mandatory
    // discard. This keeps protected singleton and payload hands from advertising an
    // impossible connector while allowing duplicate or otherwise legal costs:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/788
    // Dialga-GX is a modeled Apex Dragon payload. Under strict current-turn DCI,
    // preserve it when Crispin already covers the Supporter connector and the public
    // hand retains either Tapu recovery or a payable one-discard Regidrago V route.
    // Exactly one Crispin and no held Blender preserve Wonder Tag whenever a stronger
    // direct payload outlet already solves that axis:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/issues/653
    if (tapu_dialga_are_only_opening_basics && strict_payload_timing() &&
        dialga_has_route_preservation_value && crispin_route_already_held &&
        remove_one(state_.hand, Card::TapuLeleGX)) {
      state_.active = Pokemon{Card::TapuLeleGX, 0};
      outcome_.started_regi = false;
      return;
    }

    const bool no_lock_oricorio_dialga_are_only_opening_basics =
        hand_count(Card::Oricorio) > 0 && hand_count(Card::DialgaGX) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::TapuLeleGX) == 0;
    const bool no_lock_dialga_is_redundant_payload =
        hand_count(Card::DialgaGX) == 1 && payloads_in_hand > 1;
    const bool no_lock_vital_dance_has_energy_value = !both_manual_energy_types_held;
    const bool no_lock_public_regidrago_v_route =
        hand_count(Card::RegidragoVstar) > 0 &&
        (hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0 ||
         hand_count(Card::Arven) > 0 ||
         hand_count(Card::StevensResolve) > 0);

    // Setup may start a redundant Dialga-GX so Oricorio remains in hand and can
    // later be played onto the Bench for Vital Dance. Apply the no-lock route only
    // under strict JIT, while another held Dragon covers payload timing, at least
    // one GGF Energy type is missing, Regidrago VSTAR is held, and the public hand
    // exposes Regidrago V through a direct one-discard Item, Arven, or Steven's Resolve:
    // Setup procedure: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression evidence: https://github.com/FlareZ123/pokemon-sims/issues/663
    if (no_lock_oricorio_dialga_are_only_opening_basics &&
        scenario_.locks == LockMode::None && scenario_.dci == DciProfile::StrictJit &&
        no_lock_dialga_is_redundant_payload && no_lock_vital_dance_has_energy_value &&
        no_lock_public_regidrago_v_route && remove_one(state_.hand, Card::DialgaGX)) {
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

  bool should_setup_bench_redundant_tapu() const {
    const int held_supporters = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
    const bool direct_supporter_and_recovery_graph =
        hand_count(Card::Crispin) > 0 && hand_count(Card::Gladion) > 0 &&
        held_supporters >= 3;
    const bool manual_energy_route_held =
        hand_count(Card::Grass) > 0 || hand_count(Card::Fire) > 0;
    const bool dragon_payload_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool opening_route_unlocked = scenario_.locks == LockMode::None;

    // Setup may place an additional Basic Pokemon onto the Bench before Prize cards
    // are set. Tapu Lele-GX's Wonder Tag does not trigger during setup, so sacrifice
    // that later connector only when the public hand already has saturated Supporter,
    // Energy, Prize-recovery, and Dragon-payload routes behind Active Regidrago V:
    // Setup procedure: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mega Dragonite ex payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Lock scenarios: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-scenarios
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/671
    return active_is_regi_v() && scenario_.dci == DciProfile::StrictJit &&
           opening_route_unlocked && direct_supporter_and_recovery_graph &&
           manual_energy_route_held && dragon_payload_held;
  }

  void choose_opening_bench() {
    if (bench_space() > 0 && hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) < 2) {
      remove_one(state_.hand, Card::RegidragoV);
      state_.bench.push_back(Pokemon{Card::RegidragoV, 0});
    }

    const bool held_shuffle_draw_can_use_hand_compression = hand_count(Card::TateLiza) > 0;
    const bool dense_regi_opening_can_use_hand_compression =
        active_is_regi_v() && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 && hand_count(Card::Grass) > 0 &&
        hand_count(Card::Fire) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool latias_has_opening_bench_value =
        !active_is_regi_v() || held_shuffle_draw_can_use_hand_compression ||
        dense_regi_opening_can_use_hand_compression;

    // Setup-benching Latias ex is legal even when Regidrago V starts Active. Preserve
    // it in hand for generic Regidrago openings, while allowing the public routes that
    // gain hand compression before Tate & Liza or already hold Steven, Blender, both
    // Energy types, and a Dragon payload:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/657
    if (bench_space() > 0 && hand_count(Card::LatiasEx) > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx) &&
        latias_has_opening_bench_value) {
      remove_one(state_.hand, Card::LatiasEx);
      state_.bench.push_back(Pokemon{Card::LatiasEx, 0});
    }
    if (bench_space() > 0 && hand_count(Card::TapuLeleGX) > 0 &&
        !in_play(Card::TapuLeleGX) && should_setup_bench_redundant_tapu()) {
      remove_one(state_.hand, Card::TapuLeleGX);
      state_.bench.push_back(Pokemon{Card::TapuLeleGX, 0});
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

  void resolve_gladion_prize_exchange(const Card target) {
    // Gladion is played from hand, lets the player inspect every face-down Prize,
    // puts one chosen Prize into hand, then shuffles Gladion into the remaining
    // Prize cards. Keep that state transition in one resolver so route policies
    // only decide which legally revealed target to take.
    // Gladion card data: https://api.pokemontcg.io/v2/cards/sm4-95
    // Official Supporter and Prize procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Advanced procedure transcription: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    if (!remove_one(state_.hand, Card::Gladion)) {
      throw std::logic_error("Gladion disappeared before Prize exchange");
    }
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto prize = std::find(state_.prizes.begin(), state_.prizes.end(), target);
    if (prize == state_.prizes.end()) {
      throw std::logic_error("Gladion target disappeared after Prize reveal");
    }
    *prize = Card::Gladion;
    state_.hand.push_back(target);
    shuffle(state_.prizes);
  }

  bool known_t2_steven_crispin_latias_route_available() const {
    if (!prizes_known() || state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.max_turn < 2 || !supporter_allowed() ||
        hand_count(Card::Gladion) == 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::Crispin) > 0 || hand_count(Card::LatiasEx) > 0 ||
        prize_count_after_reveal(Card::Oricorio) == 0 ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || !payload_ready() ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    // Steven's Resolve supplies the three unresolved card axes on T1. On T2,
    // Crispin plus the legal manual attachment add one Grass and one Fire,
    // ordinary evolution supplies Regidrago VSTAR, and Skyliner gives the Basic
    // Active no Retreat Cost. The no-discard-control payload is already banked:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/1191
    return std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn <= state_.turn && pokemon.grass == 1 &&
              pokemon.fire == 0;
        });
  }

  // This historical function name is part of the surrounding macro composition chain.
  bool play_gladion_issue1204_original() {
    if (!known_t2_steven_crispin_latias_route_available()) {
      return play_gladion_issue1191_original();
    }

    // Recovering prized Oricorio spends the only T1 Supporter and advances only
    // the Energy axis. Preserve Gladion and let the held Steven package complete
    // Evolution, Energy, and Active position on the earliest legal ready turn:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://github.com/FlareZ123/pokemon-sims/issues/1191
    if (trace_ != nullptr) {
      trace_->add_policy_once(
          state_, "issue-1191-steven-before-prized-oricorio", state_.turn,
          "HOLD SUPPORTER", "R-GLADION-01; P-COMPRESS-01; P-KNOWLEDGE-01",
          "Retained Gladion because held Steven's Resolve has the complete T2 VSTAR, Crispin, and Latias package.");
    }
    return false;
  }

  bool blind_gladion_missing_regi_route_admitted() const {
    if (prizes_known() || !supporter_allowed() || hand_count(Card::Gladion) == 0 ||
        !need_regi() || bench_space() == 0 || hand_count(Card::RegidragoV) > 0) {
      return false;
    }

    const bool payable_direct_search = !item_locked() &&
        is_legal_cost_available(can_play_payload_this_turn(), true) &&
        (hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0);
    const bool held_arven_route = !item_locked() && hand_count(Card::Arven) > 0;

    // Mirror the existing blind missing-Basic admission. The decision to play
    // Gladion uses only the public hand and field, before any Prize identity is read:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
    return !payable_direct_search && hand_count(Card::UltraBall) == 0 &&
        !held_arven_route;
  }

  bool prized_treasure_regidrago_route_after_legal_reveal() const {
    if (!blind_gladion_missing_regi_route_admitted()) return false;

    Engine projected = *this;
    if (!remove_one(projected.state_.hand, Card::Gladion)) return false;
    projected.state_.supporter_used = true;
    projected.prizes_revealed_ = true;

    if (projected.item_locked() || projected.bench_space() == 0 ||
        projected.prize_count_after_reveal(Card::RegidragoV) > 0 ||
        projected.prize_count_after_reveal(Card::MysteriousTreasure) == 0 ||
        projected.deck_count_after_search_started(Card::RegidragoV) == 0) {
      return false;
    }

    projected.state_.hand.push_back(Card::MysteriousTreasure);
    // After Gladion legally reveals every Prize, the searched Item and its remaining
    // discard cost are public. Require an actual cost after Gladion leaves the hand:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core Supporter, Item, discard, search, and Bench procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Immediate missing-axis priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
    return projected.choose_discard(projected.can_play_payload_this_turn(), true,
                                    true, Card::MysteriousTreasure).has_value();
  }

  // Unified tests intentionally call the historical helper names as a source-contract seam.
  // Test owner: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1204_gladion_treasure_tests.cpp
  bool issue_1204_blind_gladion_missing_regi_admitted() const {
    return blind_gladion_missing_regi_route_admitted();
  }

  bool issue_1204_prized_treasure_route_after_legal_reveal() const {
    return prized_treasure_regidrago_route_after_legal_reveal();
  }

  bool play_gladion() {
    if (!prized_treasure_regidrago_route_after_legal_reveal()) {
      return play_gladion_issue1204_original();
    }

    resolve_gladion_prize_exchange(Card::MysteriousTreasure);

    // The revealed Treasure is an immediate, payable Regidrago V connector. It
    // precedes the inert Dipplin fallback while direct prized Regidrago recovery and
    // every other established Gladion target remain delegated to the prior policy:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Dipplin: https://api.pokemontcg.io/v2/cards/sv6-127
    // Core Supporter and Prize exchange procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
    trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Looked at Prize cards and exchanged Gladion for Mysterious Treasure to establish the missing Regidrago V route.");
    return true;
  }

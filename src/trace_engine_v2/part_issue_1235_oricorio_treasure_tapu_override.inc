  bool issue_1235_t1_quick_ball_connector_state() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 1 ||
        scenario_.going_first || scenario_.locks != LockMode::None ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        bench_space() < 2 || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::RegidragoV) == 0 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Oricorio) > 0 || in_play(Card::Oricorio) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn;
        });
    const auto cost = choose_discard(false, true, true, Card::QuickBall);
    return prior_turn_benched_regi && cost == Card::RegidragoV &&
        might_be_unseen(Card::Oricorio) && might_be_unseen(Card::TapuLeleGX);
  }

  bool issue_1235_oricorio_route_available_after_quick_ball_search() const {
    if (!deck_seen_ || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    // Vital Dance must leave one Grass and one Fire for the following turn's
    // Crispin search. Mysterious Treasure then discards the held Dragon payload,
    // searches Tapu Lele-GX, and Wonder Tag obtains Crispin. This comparison is
    // made only after Quick Ball legally exposes the deck, so it does not read
    // future draws or unrevealed Prize identities:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, Bench, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    return deck_count_after_search_started(Card::Oricorio) > 0 &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) >= 2;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (!issue_1235_t1_quick_ball_connector_state()) {
      return play_quick_ball_issue1235_original(permit_payload);
    }

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(Card::RegidragoV, "Quick Ball cost", "R-QB-01; P-DCI-01");
    record_deck_search_knowledge("Quick Ball");

    Card selected = Card::TapuLeleGX;
    if (issue_1235_oricorio_route_available_after_quick_ball_search()) {
      selected = Card::Oricorio;
    } else if (deck_count_after_search_started(Card::TapuLeleGX) == 0) {
      selected = fallback_quick_ball_target_after_search_started();
    }
    const bool found = move_deck_to_hand(selected);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
          found ? "Discarded a redundant Regidrago V and searched the fastest K1 Basic connector: " +
                      std::string(name(selected)) + "."
                : "Discarded a redundant Regidrago V; no Basic Pokemon remained after search.");
    return true;
  }

  bool issue_1235_t2_treasure_tapu_context_available() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 2 ||
        scenario_.locks != LockMode::None || !deck_seen_ ||
        !supporter_allowed() || state_.supporter_used || bench_space() == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        (hand_count(Card::RegidragoVstar) == 0 && !active_is_vstar()) ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Crispin) > 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !state_.active ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass + state_.active->fire < 1 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }
    return state_.active->grass >= 1 &&
        state_.active->grass + deck_count_after_search_started(Card::Grass) >= 2 &&
        state_.active->fire + hand_count(Card::Fire) +
                deck_count_after_search_started(Card::Fire) >= 1;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (!permit_payload || !issue_1235_t2_treasure_tapu_context_available()) {
      return play_mysterious_treasure_issue1235_original(permit_payload);
    }

    const auto cost = choose_discard(true, true, true, Card::MysteriousTreasure);
    if (!cost || !is_payload(*cost)) return false;
    const Card discarded_payload = *cost;

    // Mysterious Treasure may pay the current-turn Dragon payload because the
    // searched Tapu Lele-GX obtains Crispin, which deterministically completes GGF:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, earliest-route, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(discarded_payload,
                      "Mysterious Treasure cost", "R-MT-01; P-JIT-01");
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error(
          "projected issue-1235 Mysterious Treasure Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Discarded the current-turn Dragon payload and searched Tapu Lele-GX for the Crispin T2 route.");
    return true;
  }

  bool issue_1235_t2_treasure_tapu_crispin_completion_available() const {
    if (!issue_1235_t2_treasure_tapu_context_available()) return false;
    Engine projected = *this;
    projected.trace_ = nullptr;
    if (!projected.play_mysterious_treasure(true)) return false;
    projected.play_basics_from_hand();
    if (projected.hand_count(Card::Crispin) == 0 ||
        !projected.play_crispin()) return false;
    projected.attach_manual();
    projected.evolve_best_regi();
    return projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  void choose_supporter() {
    if (!issue_1235_t2_treasure_tapu_crispin_completion_available()) {
      choose_supporter_issue1235_original();
      return;
    }

    // Preserve the Supporter play for Treasure -> Tapu Lele-GX -> Crispin:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    if (trace_ != nullptr) {
      trace_->add_policy_once(
          state_, "issue-1235-treasure-tapu-crispin-before-supporter", state_.turn,
          "HOLD SUPPORTER", "R-MT-01; R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Retained the Supporter play for the proven Active-Regidrago Treasure to Tapu Lele-GX to Crispin T2 finish.");
    }
  }

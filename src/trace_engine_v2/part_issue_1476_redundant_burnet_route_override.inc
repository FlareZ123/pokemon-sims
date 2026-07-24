  bool issue_1476_second_wonder_tag_route_available() const {
    const bool active_vstar_route = state_.active &&
        ((state_.active->card == Card::RegidragoVstar) ||
         (state_.active->card == Card::RegidragoV &&
          state_.active->entered_turn < state_.turn &&
          hand_count(Card::RegidragoVstar) > 0));

    // After the first Wonder Tag banks Crispin, Quick Ball may spend Latias ex and
    // fetch the second physical Tapu Lele-GX. The second Wonder Tag banks the second
    // Crispin, so turns two and three deterministically supply GGF while Blender
    // supplies the strict-JIT payload on the ready turn:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Bench, Supporter, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
    return deck_seen_ && scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn == 2 &&
        scenario_.going_first && scenario_.max_turn >= 3 && !item_locked() &&
        active_vstar_route && supporter_allowed() &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
        count_of(state_.discard, Card::ProfessorBurnet) > 0 &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 && !payload_ready() &&
        in_play_count(Card::TapuLeleGX) == 1 && bench_space() > 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) >= 2 &&
        payload_might_be_in_deck();
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (!issue_1476_redundant_burnet_route_available(
            Card::MysteriousTreasure)) {
      return play_mysterious_treasure_issue1476_original(permit_payload);
    }

    // Mysterious Treasure pays its printed cost before the first legal inspection.
    // The complete public route makes Burnet the lowest-DCI cost, and Tapu Lele-GX
    // is the advancing Psychic target. Benching it immediately is a separate legal
    // game action after the Item resolves and banks the first Crispin for turn two:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Official first-turn, Item, search, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::ProfessorBurnet,
                           "Mysterious Treasure cost", "R-MT-01; P-DCI-01")) {
      throw std::logic_error("projected issue-1476 Burnet cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");

    const bool found_tapu = move_deck_to_hand(Card::TapuLeleGX);
    const Card fallback = found_tapu
        ? Card::TapuLeleGX
        : fallback_mysterious_target_after_search_started();
    const bool found_fallback = found_tapu || move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          found_fallback
              ? "Spent redundant Professor Burnet and searched " +
                    std::string(name(fallback)) + "."
              : "Spent redundant Professor Burnet; no Psychic or Dragon Pokemon remained after search.");

    if (found_tapu && !bench_from_hand(Card::TapuLeleGX, true)) {
      throw std::logic_error("projected issue-1476 first Tapu Bench action disappeared");
    }
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (!issue_1476_second_wonder_tag_route_available()) {
      return play_quick_ball_issue1476_original(permit_payload);
    }

    // Active Regidrago has replaced Latias ex's movement role. Quick Ball spends
    // that now-inert singleton, searches the second physical Tapu Lele-GX, and
    // Benches it so Wonder Tag banks the second Crispin before the T2 Supporter play:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Official Item, discard, search, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(Card::LatiasEx, "Quick Ball cost",
                           "R-QB-01; P-DCI-01")) {
      throw std::logic_error("projected issue-1476 Latias cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("projected issue-1476 second Tapu target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-DCI-01",
          "Discarded redundant Latias ex and searched the second Tapu Lele-GX.");
    if (!bench_from_hand(Card::TapuLeleGX, true)) {
      throw std::logic_error("projected issue-1476 second Tapu Bench action disappeared");
    }
    return true;
  }

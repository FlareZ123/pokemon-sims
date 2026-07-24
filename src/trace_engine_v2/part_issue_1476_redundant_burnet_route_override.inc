  bool issue_1476_should_hold_quick_ball_for_oricorio_route() const {
    if (!deck_seen_ || !state_.active) return false;

    const bool crispin_route_is_live = hand_count(Card::Crispin) > 0 ||
        (state_.supporter_used && count_of(state_.discard, Card::Crispin) > 0);
    const int reachable_grass = state_.active->grass + hand_count(Card::Grass) +
        deck_count_after_search_started(Card::Grass);
    const int reachable_fire = state_.active->fire + hand_count(Card::Fire) +
        deck_count_after_search_started(Card::Fire);

    // The T1 Treasure search has established K1 and banked Crispin through Wonder
    // Tag. Quick Ball must stay in hand during T2's early Item windows because
    // Latias ex becomes legal dynamic-DCI fuel only after the Active Regidrago V
    // evolves in place. The later Item window then reuses the existing proven
    // Quick Ball -> Oricorio -> Vital Dance selector:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Bench, Supporter, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn == 2 &&
        scenario_.going_first && scenario_.max_turn >= 3 && !item_locked() &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 && !payload_ready() &&
        crispin_route_is_live && in_play_count(Card::TapuLeleGX) == 1 &&
        hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio) &&
        deck_count_after_search_started(Card::Oricorio) > 0 &&
        reachable_grass >= 2 && reachable_fire >= 1 &&
        payload_might_be_in_deck();
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (!issue_1476_redundant_burnet_route_available(
            Card::MysteriousTreasure)) {
      return play_mysterious_treasure_issue1476_original(permit_payload);
    }

    // Mysterious Treasure pays its printed cost before the first legal inspection.
    // Held Blender strictly covers Burnet's remaining payload role, while Tapu
    // Lele-GX is the advancing Psychic target. Benching it after the Item resolves
    // legally establishes Wonder Tag and banks Crispin for turn two:
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
      throw std::logic_error("projected issue-1476 Tapu Bench action disappeared");
    }
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1476_should_hold_quick_ball_for_oricorio_route()) {
      // Waiting preserves Quick Ball and Latias ex until evolution makes Skyliner
      // inert. The post-evolution call delegates to the already source-backed K1
      // Oricorio selector and pays the printed cost before the new search:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/sm2-55
      // https://github.com/FlareZ123/pokemon-sims/issues/1476
      return false;
    }
    return play_quick_ball_issue1476_original(permit_payload);
  }

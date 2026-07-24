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
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
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
    // The refined route leaves Quick Ball to the existing adaptive K1 selector.
    // It may search Oricorio only when the inspected Energy state still needs Vital
    // Dance; seed 129 holds Quick Ball because its T2 draw plus Crispin already
    // establishes GG and makes the extra discard/search sequence unnecessary:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
    return play_quick_ball_issue1476_original(permit_payload);
  }

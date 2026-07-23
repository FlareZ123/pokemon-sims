  bool issue_1411_t1_treasure_tapu_steven_route_available() {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        scenario_.going_first || scenario_.max_turn < 2 || item_locked() ||
        rule_box_abilities_locked() || !supporter_allowed() ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV || bench_space() <= 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::FieldBlower) == 0 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::Grass) < 2 || hand_count(Card::Fire) == 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !needs_tapu_connector()) {
      return false;
    }

    // The T1 attachment remains available before Steven's Resolve ends the turn.
    // Steven banks VSTAR, Crispin, and Brilliant Blender; the second held Grass,
    // Crispin's Fire attachment, and the T2 manual Grass attachment complete GGF.
    // Field Blower has no modeled Tool or Stadium target in this no-lock state:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Official Item, Bench, Ability, Supporter, attachment, search, and evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1411
    return might_be_unseen(Card::TapuLeleGX) &&
        might_be_unseen(Card::StevensResolve) &&
        might_be_unseen(Card::RegidragoVstar) &&
        might_be_unseen(Card::Crispin) &&
        might_be_unseen(Card::BrilliantBlender) &&
        might_be_unseen(Card::Grass) && might_be_unseen(Card::Fire) &&
        payload_might_be_in_deck();
  }

  bool issue_1411_play_treasure_tapu_steven_route() {
    if (!issue_1411_t1_treasure_tapu_steven_route_available()) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::FieldBlower,
                           "Mysterious Treasure T1 Steven-route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1411 Field Blower cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    const bool found_tapu = move_deck_to_hand(Card::TapuLeleGX);
    if (!found_tapu) {
      const Card fallback = fallback_mysterious_target_after_search_started();
      move_deck_to_hand(fallback);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          found_tapu
              ? "Discarded setup-dead Field Blower and searched Tapu Lele-GX for the complete T1 Steven compression route."
              : "Tapu Lele-GX was unavailable after search; resolved Mysterious Treasure with a legal fallback target.");
    return true;
  }

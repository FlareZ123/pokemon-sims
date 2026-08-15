  bool issue_1817_prelock_route_pending_{false};
  bool issue_1817_burnet_finish_pending_{false};

  bool issue_1817_prelock_treasure_tapu_route_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::TurnTwoItem || state_.turn != 1 ||
        scenario_.going_first || scenario_.max_turn < 3 || !deck_seen_ ||
        item_locked() || rule_box_abilities_locked() || !supporter_allowed() ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV || bench_space() <= 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Dipplin) == 0 || hand_count(Card::Grass) == 0 ||
        hand_count(Card::Fire) == 0 || hand_count(Card::TapuLeleGX) > 0 ||
        in_play(Card::TapuLeleGX) || hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::ProfessorBurnet) > 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX)) {
      return false;
    }
    return deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0;
  }

  bool issue_1817_play_prelock_treasure_tapu_route() {
    if (!issue_1817_prelock_treasure_tapu_route_available()) return false;
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::Dipplin,
                           "Mysterious Treasure pre-lock Tapu cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1817 Dipplin cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("projected issue-1817 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Mysterious Treasure discarded Dipplin and searched Tapu Lele-GX "
          "for the pre-lock Steven and Professor Burnet route.");

    // Spend the visible Fire attachment before Steven ends T1. The held Grass
    // plus Steven's searched Grass then cover the ordinary T2 and T3 manual
    // attachments through the scheduled Item lock:
    // Basic Energy attachment: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Scheduled lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1817
    if (!remove_one(state_.hand, Card::Fire)) {
      throw std::logic_error("projected issue-1817 Fire attachment disappeared");
    }
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY", "Fire Energy manually to Regidrago V.");
    issue_1817_prelock_route_pending_ = true;
    return true;
  }

  bool issue_1817_play_prelock_steven_package() {
    if (!issue_1817_prelock_route_pending_ || !supporter_allowed() ||
        hand_count(Card::StevensResolve) == 0 || !deck_seen_ || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->grass != 0 || state_.active->fire != 1 ||
        hand_count(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::ProfessorBurnet) == 0) {
      return false;
    }

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");
    for (const Card card : {Card::RegidragoVstar, Card::Grass,
                            Card::ProfessorBurnet}) {
      if (!move_deck_to_hand(card)) {
        throw std::logic_error("projected issue-1817 Steven target disappeared");
      }
    }
    shuffle(state_.deck);
    issue_1817_prelock_route_pending_ = false;
    issue_1817_burnet_finish_pending_ = true;

    // The three searched cards solve evolution, the final Basic Energy, and the
    // current-turn T3 payload outlet without relying on Items after T1:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, evolution, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1817
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched up to 3 cards: Regidrago VSTAR, Grass Energy, Professor Burnet. Turn ended.");
    return true;
  }

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

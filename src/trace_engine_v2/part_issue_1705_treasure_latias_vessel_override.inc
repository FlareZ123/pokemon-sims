bool play_issue_1705_t1_treasure_latias_vessel_route() {
  const bool public_entry =
      scenario_.dci == DciProfile::StrictJit &&
      scenario_.locks == LockMode::None && !scenario_.going_first &&
      state_.turn == 1 && supporter_allowed() && !item_locked() &&
      !state_.manual_energy_used && state_.active &&
      state_.active->card == Card::CrobatV &&
      state_.active->entered_turn < state_.turn && !has_any_regi() &&
      !in_play(Card::LatiasEx) && bench_space() >= 2 &&
      hand_count(Card::QuickBall) > 0 &&
      hand_count(Card::TeamYellsCheer) > 0 &&
      hand_count(Card::MysteriousTreasure) > 0 &&
      hand_count(Card::FieldBlower) > 0 &&
      hand_count(Card::EarthenVessel) > 0 &&
      hand_count(Card::Dragapult) > 0 &&
      hand_count(Card::StevensResolve) > 0;
  if (!public_entry) return false;

  // Quick Ball is already the public K0-optimal Regidrago connector. Resolve that
  // ordinary action first, then use its legal deck inspection to establish K1 before
  // evaluating the two route-replaced strict-JIT costs:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1705
  remove_one(state_.hand, Card::QuickBall);
  state_.discard.push_back(Card::QuickBall);
  if (!discard_from_hand(Card::TeamYellsCheer,
                         "Quick Ball issue-1705 Regidrago cost",
                         "R-QB-01; P-DCI-01")) {
    throw std::logic_error("Issue-1705 Quick Ball cost disappeared");
  }
  record_deck_search_knowledge("Quick Ball issue-1705 route");
  const bool found_regidrago = move_deck_to_hand(Card::RegidragoV);
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01",
        found_regidrago
            ? "Discarded Team Yell's Cheer and searched Regidrago V."
            : "Discarded Team Yell's Cheer; no Regidrago V remained in deck.");
  if (!found_regidrago) return false;

  const bool complete_k1_route =
      bench_space() >= 2 &&
      ability_available_for_pokemon(Card::LatiasEx) &&
      deck_count_after_search_started(Card::LatiasEx) > 0 &&
      deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
      deck_count_after_search_started(Card::Crispin) > 0 &&
      deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
      deck_count_after_search_started(Card::Grass) >= 2 &&
      deck_count_after_search_started(Card::Fire) >= 2 &&
      payload_might_be_in_deck();
  if (!complete_k1_route) return false;

  // K1 proves that singleton Field Blower has no modeled role in this lock-free
  // setup and that Brilliant Blender deterministically replaces the early Dragapult
  // payload on the T2 ready turn. These exact replacement proofs make both costs
  // admissible under strict JIT while every incomplete route preserves the cards:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1705
  remove_one(state_.hand, Card::MysteriousTreasure);
  state_.discard.push_back(Card::MysteriousTreasure);
  if (!discard_from_hand(Card::FieldBlower,
                         "Mysterious Treasure issue-1705 route-replaced cost",
                         "R-MT-01; P-DCI-01; P-COMPRESS-01") ||
      !move_deck_to_hand(Card::LatiasEx)) {
    throw std::logic_error("Issue-1705 Treasure route disappeared after K1 proof");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-KNOWLEDGE-01",
        "Discarded setup-replaced Field Blower and searched Latias ex.");

  if (!bench_from_hand(Card::RegidragoV, false) ||
      !bench_from_hand(Card::LatiasEx, false)) {
    throw std::logic_error("Issue-1705 Bench route disappeared after K1 proof");
  }

  remove_one(state_.hand, Card::EarthenVessel);
  state_.discard.push_back(Card::EarthenVessel);
  if (!discard_from_hand(Card::Dragapult,
                         "Earthen Vessel issue-1705 Blender-replaced payload cost",
                         "R-EV-01; P-DCI-01; P-COMPRESS-01") ||
      !move_deck_to_hand(Card::Grass) || !move_deck_to_hand(Card::Fire)) {
    throw std::logic_error("Issue-1705 Vessel route disappeared after K1 proof");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-DCI-01",
        "Discarded Blender-replaced Dragapult ex and searched Grass Energy, Fire Energy.");

  Pokemon* regidrago = target_regi();
  if (regidrago == nullptr || regidrago->card != Card::RegidragoV ||
      !remove_one(state_.hand, Card::Grass)) {
    throw std::logic_error("Issue-1705 T1 Grass attachment target disappeared");
  }
  ++regidrago->grass;
  state_.manual_energy_used = true;
  trace("ATTACH", "R-GAME-ATTACH; P-COMPRESS-01",
        "Grass Energy manually to Regidrago V before Steven's Resolve.");

  remove_one(state_.hand, Card::StevensResolve);
  state_.discard.push_back(Card::StevensResolve);
  state_.supporter_used = true;
  outcome_.used_steven = true;
  if (!move_deck_to_hand(Card::RegidragoVstar) ||
      !move_deck_to_hand(Card::Crispin) ||
      !move_deck_to_hand(Card::BrilliantBlender)) {
    throw std::logic_error("Issue-1705 Steven package disappeared after K1 proof");
  }
  shuffle(state_.deck);
  state_.turn_ended = true;
  // Steven ends T1 after banking the exact VSTAR, Energy, and current-turn payload
  // channels. The prior-turn Regidrago can evolve on T2; Crispin plus the manual
  // Fire attachment completes GGF; Latias gives Crobat V free Retreat; Blender then
  // supplies the strict-JIT payload during the ready turn:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Core turn, Supporter, attachment, evolution, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1705
  trace("PLAY SUPPORTER",
        "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Searched Regidrago VSTAR, Crispin, Brilliant Blender; turn ended after the complete T2 route.");
  return true;
}

bool late_steven_has_known_t3_compression_route_issue1705() const {
  return late_steven_has_known_t3_compression_route();
}

bool late_steven_has_known_t3_compression_route_issue1705() {
  if (play_issue_1705_t1_treasure_latias_vessel_route()) return true;
  return late_steven_has_known_t3_compression_route();
}

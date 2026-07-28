    // Confirmed replay bug: https://github.com/FlareZ123/pokemon-sims/issues/1147
    // Route-conditioned surplus Energy fallback: https://github.com/FlareZ123/pokemon-sims/issues/1146
    return choose_discard(false, true, true, Card::QuickBall).has_value() ||
           quick_ball_final_surplus_energy_cost().has_value();
  }

bool complete_issue_1564_quick_ball_latias_promotion() {
  if (scenario_.dci != DciProfile::NoDiscardControl || item_locked() ||
      !deck_seen_ || hand_count(Card::QuickBall) == 0 ||
      hand_count(Card::Grass) == 0 || !need_active_vstar() ||
      need_energy() || need_payload() || state_.retreat_used ||
      !state_.active || state_.active->card != Card::TapuLeleGX ||
      bench_space() == 0 ||
      !ability_available_for_pokemon(Card::LatiasEx) ||
      in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
      deck_count_after_search_started(Card::LatiasEx) == 0) {
    return false;
  }
  Pokemon* target = best_benched_vstar_for_promotion();
  if (target == nullptr || target->grass < 2 || target->fire < 1) {
    return false;
  }

  // The payload is already banked under no-discard-control and the Benched
  // Regidrago VSTAR already pays GGF. The remaining Grass is therefore a
  // route-proven high-DCI Quick Ball cost. Search and Bench Latias ex, then
  // Skyliner removes the Basic Active Tapu Lele-GX's Retreat Cost so the ready
  // VSTAR can be promoted during the same turn:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, discard, search, Bench, Ability, retreat, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // No-control payload banking, DCI, and shortest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1564
  remove_one(state_.hand, Card::QuickBall);
  state_.discard.push_back(Card::QuickBall);
  if (!discard_from_hand(Card::Grass,
                         "Quick Ball issue-1564 Latias route cost",
                         "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
    throw std::logic_error("Issue-1564 surplus Grass cost disappeared");
  }
  record_deck_search_knowledge("Quick Ball issue-1564 Latias route");
  if (!move_deck_to_hand(Card::LatiasEx)) {
    throw std::logic_error("Issue-1564 Latias ex target disappeared");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-DCI-01",
        "Spent surplus Grass Energy and searched Latias ex for the immediate promotion.");
  if (!bench_from_hand(Card::LatiasEx, false)) {
    throw std::logic_error("Issue-1564 Latias ex Bench action failed");
  }
  if (!retreat_to_benched_vstar_with_latias()) {
    throw std::logic_error("Issue-1564 Skyliner promotion disappeared");
  }
  return active_is_vstar() && !need_energy() && !need_payload();
}


bool complete_issue_1533_blender_treasure_latias_route() {
  const Pokemon* target = target_regi();
  if (!strict_payload_timing() || item_locked() || !prizes_known() ||
      target == nullptr || target->card != Card::RegidragoVstar ||
      target->grass < 2 || target->fire < 1 ||
      !need_active_vstar() || !need_payload() || state_.retreat_used ||
      !state_.active || state_.active->card != Card::TapuLeleGX ||
      bench_space() == 0 ||
      !ability_available_for_pokemon(Card::LatiasEx) ||
      in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
      deck_count_after_search_started(Card::LatiasEx) == 0 ||
      hand_count(Card::MysteriousTreasure) == 0 ||
      hand_count(Card::BrilliantBlender) == 0 ||
      std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
      std::none_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
    return false;
  }

  auto cost = choose_discard(false, true, true, Card::MysteriousTreasure);
  if (!cost) cost = quick_ball_final_surplus_energy_cost();
  if (!cost) return false;

  // The selected Benched Regidrago VSTAR already pays GGF. Mysterious Treasure
  // may spend the route-proven surplus Energy, search Latias ex, and Bench it.
  // Brilliant Blender then establishes the strict-JIT payload, after which
  // Skyliner promotes the completed attacker on the same turn:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
  remove_one(state_.hand, Card::MysteriousTreasure);
  state_.discard.push_back(Card::MysteriousTreasure);
  if (!discard_from_hand(*cost, "Mysterious Treasure issue-1533 Latias route cost",
                         "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
    throw std::logic_error("Issue-1533 Treasure cost disappeared");
  }
  record_deck_search_knowledge("Mysterious Treasure issue-1533 Latias route");
  if (!move_deck_to_hand(Card::LatiasEx)) {
    throw std::logic_error("Issue-1533 Latias ex target disappeared");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
        "Spent the route-proven surplus cost and searched Latias ex.");
  if (!bench_from_hand(Card::LatiasEx, false)) {
    throw std::logic_error("Issue-1533 Latias ex Bench play disappeared");
  }
  if (!play_brilliant_blender()) {
    throw std::logic_error("Issue-1533 Brilliant Blender payload disappeared");
  }
  return retreat_to_benched_vstar_with_latias();
}

  bool complete_late_steven_vstar_vessel_continuation() {
    if (scenario_.dci != DciProfile::StrictJit || item_locked() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        (deck_count_after_search_started(Card::Grass) == 0 &&

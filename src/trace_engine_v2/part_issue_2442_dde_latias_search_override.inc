  bool post_attach_treasure_latias_route_available() {
    const Pokemon* target = target_regi();
    if (!strict_payload_timing() || item_locked() || target == nullptr ||
        target->card != Card::RegidragoVstar ||
        !pays_apex_energy_cost(*target) || !need_active_vstar() ||
        state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        !might_be_unseen(Card::LatiasEx) ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // A manual attachment can complete Apex Dragon on the prior-turn Regidrago
    // VSTAR. Double Dragon Energy makes DDE + Grass and DDE + Fire complete GGF
    // payment states, so the selected attacker must use semantic Energy payment.
    // Mysterious Treasure can then discard the held Dragon, search Latias ex, and
    // Skyliner can promote the ready VSTAR on the same turn:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Bench, Ability, Energy, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2442
    return choose_discard(true, true, true, Card::MysteriousTreasure).has_value();
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
    if (target == nullptr || !pays_apex_energy_cost(*target)) return false;

    // The payload is already banked under no-discard-control and the selected
    // Benched Regidrago VSTAR already semantically pays Apex Dragon. DDE + Grass
    // and DDE + Fire therefore remain valid promotion targets. The remaining
    // Grass is route-proven Quick Ball cost, then Latias ex enables Skyliner:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, search, Bench, Ability, retreat, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-control payload banking, DCI, and shortest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1564
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2442
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
        !pays_apex_energy_cost(*target) || !need_active_vstar() ||
        !need_payload() || state_.retreat_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || bench_space() == 0 ||
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

    // The selected Benched Regidrago VSTAR already semantically pays Apex Dragon,
    // including DDE + Grass and DDE + Fire. Mysterious Treasure can spend the
    // route-proven surplus cost, Latias ex solves Active position, and Brilliant
    // Blender establishes the strict-JIT Dragon payload on the same turn:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1533
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2442
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*cost,
                           "Mysterious Treasure issue-1533 Latias route cost",
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

// Rename the three stale legacy definitions that appear later in
// part_014c_latias_bench_override.inc. The semantic implementations above keep
// every existing route gate and change only the selected VSTAR Energy proof:
// Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
// Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2442
#define post_attach_treasure_latias_route_available \
  post_attach_treasure_latias_route_available_issue2442_original
#define complete_issue_1564_quick_ball_latias_promotion \
  complete_issue_1564_quick_ball_latias_promotion_issue2442_original
#define complete_issue_1533_blender_treasure_latias_route \
  complete_issue_1533_blender_treasure_latias_route_issue2442_original

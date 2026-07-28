    return;
  }
  trace("POLICY", "P-AXIS-01", "Start: " + state_line());
  if (complete_issue_1597_banked_steven_turn()) {
    trace("TURN END", "R-STEVEN-01", state_line());
    return;
  }
  if (complete_late_steven_vstar_vessel_continuation()) return;
    play_chaotic_swell();
    play_field_blower();
    if (complete_issue_1533_blender_treasure_latias_route()) {
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // Resolve the proven K1 Steven route before generic Items can consume Quick Ball
    // and erase the public-state predicate. Steven searches any three cards and ends
    // the turn, so attaching Fire and evolving the prior-turn Regidrago V first banks
    // the exact Latias ex, Mysterious Treasure, and Mega Dragonite ex continuation:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    if (late_steven_has_known_t3_compression_route()) {
      attach_manual();
      evolve_best_regi();
      choose_supporter();
      if (state_.turn_ended) {
        trace("TURN END", "R-STEVEN-01", state_line());
        return;
      }
    }
    // The banked going-first route must resolve before generic actions can consume
    // Blender or alter its K1 continuation. Attach the second public Energy, then let
    // Steven search VSTAR, the direct third Grass, and Latias. Steven ends T2, while
    // the T3 Supporter slot remains unused for any later discrete-value action:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/946
    if (banked_steven_has_known_t3_blender_route()) {
      attach_manual();
      if (play_banked_steven_for_known_t3_blender_route()) {
        trace("TURN END", "R-STEVEN-01", state_line());
        return;
      }
    }
    // A redundant Regidrago V must not consume the final Bench slot before the
    // immediately live Latias ex promotion route:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    bench_latias_for_pending_vstar_promotion();
    // Defer only Crobat V while deterministic Basic, Item, Tool, attachment, and
    // evolution actions compress the hand. Other Basic connectors remain live:
    // https://github.com/FlareZ123/pokemon-sims/issues/1751
    issue_1751_defer_crobat_ = true;
    play_basics_from_hand();
    attach_opening_oricorio_for_known_t4_fss_route();
    if (banked_steven_has_known_t4_fss_route() &&
        play_banked_steven_for_known_t4_fss_route()) {
      trace("TURN END", "R-STEVEN-01", state_line());
      return;
    }
    // Preserve the exact #869 Mysterious Treasure payload outlet before a generic
    // Supporter route can shuffle it away:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    play_known_steven_t3_payload_outlet();
    play_items_until_stable(!strict_payload_timing());
    attach_live_fss();
    issue_1751_use_fss_if_no_pending_crobat();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
    issue_1751_defer_crobat_ = false;
    if (complete_issue_1564_quick_ball_latias_promotion()) {
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    issue_1751_defer_crobat_ = true;
    // A manual attachment can complete GGF and thereby make a held Mysterious
    // Treasure payload discard legal under strict JIT. Reopen Items and evolution
    // only for that proven same-turn route. Earlier payload banking remains rejected
    // when the selected Regidrago V still lacks part of Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1032
    const Pokemon* post_attach_target = target_regi();
    const bool post_attach_treasure_vstar_route = strict_payload_timing() &&
        !item_locked() && post_attach_target != nullptr &&
        post_attach_target->card == Card::RegidragoV &&
        post_attach_target->entered_turn < state_.turn &&
        post_attach_target->grass >= 2 && post_attach_target->fire >= 1 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        might_be_unseen(Card::RegidragoVstar) &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        choose_discard(true, true, true, Card::MysteriousTreasure).has_value();
    const bool post_attach_treasure_latias_route =
        post_attach_treasure_latias_route_available();
    if (post_attach_treasure_vstar_route || post_attach_treasure_latias_route) {
      play_items_until_stable(true);
      evolve_best_regi();
      if (post_attach_treasure_latias_route) {
        play_basics_from_hand();
        resolve_final_promotion_and_attachment();
      }
    }
    // Steven's Resolve ends the turn. Resolve a still-live Crobat connector after
    // deterministic compression and before Steven; non-ending Supporters retain
    // priority so their paid actions may compress the hand first:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

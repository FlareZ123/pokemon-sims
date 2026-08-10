  void choose_supporter() {
    const bool payload_is_only_missing_axis =
        strict_payload_timing() && need_payload() && !need_regi() &&
        !need_vstar() && !need_energy() && !need_active_vstar() &&
        can_play_payload_this_turn();
    const bool held_dragon =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool live_item_payload_outlet =
        !item_locked() && held_dragon &&
        (has_live_blender_payload_line() ||
         has_live_one_discard_hand_payload_line() ||
         has_live_ultra_ball_hand_payload_line());

    // When a live Item can pay with the already-held Dragon and complete the sole
    // same-turn payload axis, preserve the entire Supporter action for later use.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Same-turn JIT and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing systemic boundary: https://github.com/FlareZ123/pokemon-sims/issues/1341
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2408
    if (payload_is_only_missing_axis && live_item_payload_outlet) return;

    // At K1, Burnet and Serena can each complete the same remaining payload axis.
    // Prefer Burnet only when its deck-resident Dragon route is proven live and no
    // Item route above can preserve the Supporter action. This keeps Serena and the
    // held Dragon available as discrete later-game resources.
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter/search/discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
    const bool burnet_preserves_serena_and_held_dragon =
        payload_is_only_missing_axis && prizes_known() && held_dragon &&
        hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::Serena) > 0 &&
        professor_burnet_has_live_ready_turn_route();
    if (burnet_preserves_serena_and_held_dragon && play_professor_burnet()) return;

    choose_supporter_issue2408_original();
  }

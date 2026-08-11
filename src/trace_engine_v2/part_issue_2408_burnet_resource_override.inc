  bool issue_2408_live_item_payload_outlet() const {
    if (item_locked() || !need_payload() || !can_play_payload_this_turn()) {
      return false;
    }

    // Reuse the established #1341 legality predicates. These already account for
    // printed discard costs, legal search targets, K1 knowledge, and whether the
    // Item can establish the current-turn Dragon payload. A live Item route is
    // strictly cheaper than either Supporter because it preserves the one
    // Supporter action for another axis or future value.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1, dynamic DCI, strict JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Established Item-outlet boundary: https://github.com/FlareZ123/pokemon-sims/issues/1341
    // Confirmed Burnet-vs-Serena bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
    return has_live_blender_payload_line() ||
        has_live_one_discard_hand_payload_line() ||
        has_live_ultra_ball_hand_payload_line();
  }

  void choose_supporter() {
    const bool payload_is_only_missing_axis =
        strict_payload_timing() && need_payload() && !need_regi() &&
        !need_vstar() && !need_energy() && !need_active_vstar() &&
        can_play_payload_this_turn();
    const bool held_dragon =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool burnet_and_serena_equal_turn_finish =
        payload_is_only_missing_axis && prizes_known() && held_dragon &&
        hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::Serena) > 0 &&
        professor_burnet_has_live_ready_turn_route();

    if (burnet_and_serena_equal_turn_finish) {
      // If an Item can finish the payload axis, preserve the Supporter action
      // entirely. Otherwise Burnet reaches the same earliest ready turn as Serena
      // while preserving Serena and the held Dragon as higher-value resources.
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Supporter, Item, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Advanced Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      // Resource priority and supporter contention: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
      if (issue_2408_live_item_payload_outlet()) return;
      if (play_professor_burnet()) return;
    }

    choose_supporter_issue2408_original();
  }

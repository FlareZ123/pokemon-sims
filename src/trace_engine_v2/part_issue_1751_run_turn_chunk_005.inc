    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
    issue_1751_defer_crobat_ = false;
    const bool issue_1751_steven_will_end_turn = should_play_steven();
    if (issue_1751_steven_will_end_turn) {
      issue_1751_resolve_crobat_after_compression(!strict_payload_timing());
      attach_powerglass();
    } else {
      issue_1751_defer_crobat_ = true;
    }
    choose_supporter();
    if (state_.turn_ended) {
      issue_1751_defer_crobat_ = false;
      trace("TURN END", "R-STEVEN-01 or R-RV-01", state_line());
      return;
    }

    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    attach_live_fss();
    issue_1751_use_fss_if_no_pending_crobat();
    play_chaotic_swell();
    play_field_blower();
    play_items_until_stable(!strict_payload_timing());
    attach_manual();
    evolve_best_regi();
    play_basics_from_hand();
    play_items_until_stable(true);
    // A strict-JIT Item route may now discard an in-hand Dragon payload and search
    // Regidrago VSTAR through Mysterious Treasure (https://api.pokemontcg.io/v2/cards/sm6-113).
    // Retry evolution so a Regidrago V that entered play on a prior turn can evolve
    // before this turn's ready check under the core evolution procedure (https://www.pokemon.com/us/pokemon-tcg/rules).
    evolve_best_regi();
    issue_1751_defer_crobat_ = false;
    if (complete_issue_1599_quick_ball_tapu_crispin_route()) {
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    if (late_steven_active_vstar_crispin_treasure_route_available() &&
        play_late_steven_active_vstar_crispin_treasure_route()) {
      // Steven's Resolve ends the turn immediately after the two-card route search.
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://github.com/FlareZ123/pokemon-sims/issues/1098
      trace("TURN END", "R-STEVEN-01", state_line());
      return;
    }
    issue_1751_defer_crobat_ = true;
    // A late Item search can put Tapu Lele-GX into hand, and Bench play can then
    // fetch a new Supporter with Wonder Tag. Reconsider that unused Supporter slot
    // before the ready check: https://api.pokemontcg.io/v2/cards/sm2-60
    // Professor Burnet can discard a current-turn deck payload after this chain:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool late_supporter_was_unused = !state_.supporter_used;
    if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    if (late_supporter_was_unused &&
        late_supporter_quick_ball_latias_completion_available()) {
      play_items_until_stable(true);
      play_basics_from_hand();
    }
    if (use_legacy_star()) {
      // Core procedure source: https://www.pokemon.com/us/pokemon-tcg/rules
      // Recovered Items may be played later in the same turn.
      play_chaotic_swell();
      play_field_blower();
      play_basics_from_hand();
      play_items_until_stable(true);
      // Legacy Star can return a Supporter from discard, including Crispin:
      // https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv7-133
      // A player may use one available Supporter during the turn before attacking:
      // https://www.pokemon.com/us/pokemon-tcg/rules
      if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
      // A Supporter recovered by Legacy Star may search an Item that is immediately
      // legal later in the same turn. Reopen the Item fixed point after Arven so a
      // searched Brilliant Blender can establish the current-turn payload before
      // the ready-state check:
      // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Core Supporter and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1565
      if (!state_.turn_ended) {
        play_chaotic_swell();
        play_field_blower();
        play_basics_from_hand();
        play_items_until_stable(true);
      }
    }
    // All deterministic non-ending actions have now had their compression window.
    // Resolve Crobat, then use any newly exposed Supporter and replay its immediate
    // Item/evolution continuation before final promotion and readiness:
    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Core turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
    issue_1751_defer_crobat_ = false;
    const bool issue_1751_crobat_resolved =
        issue_1751_resolve_crobat_after_compression(true);
    if (!issue_1751_crobat_resolved) use_fss();
    if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    if (state_.turn_ended) {
      trace("TURN END", "R-STEVEN-01 or R-RV-01", state_line());
      return;
    }
    issue_1751_defer_crobat_ = true;
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(true);
    attach_live_fss();
    use_fss();
    attach_manual();
    evolve_best_regi();
    issue_1751_defer_crobat_ = false;
    resolve_final_promotion_and_attachment();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    // Attach only after the final modeled switch decision, while the intended holder
    // is known to remain Active for Powerglass's end-of-turn condition:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    attach_powerglass();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
  }


#include "part_issue_1751_crobat_compression.inc"

  bool issue_1751_recipe_has_crobat() const {
    // Keep the registered shell and Pineco action selectors on the authoritative
    // current-main policy. The compression-first turn selector is recipe-gated to
    // experimental Crobat V lists:
    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Modeling boundary: https://github.com/FlareZ123/pokemon-sims/issues/1394
    // Confirmed sequencing bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
    return std::any_of(recipe_.begin(), recipe_.end(), [](const auto& entry) {
      return entry.first == Card::CrobatV && entry.second > 0;
    });
  }

void run_turn_issue_1751_core() {
  if (secret_box_combo_enabled()) {
    run_secret_box_turn();
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
        pays_apex_energy_cost(*post_attach_target) && // DDE-aware: https://github.com/FlareZ123/pokemon-sims/issues/2368
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


  void run_turn() {
    if (!issue_1751_recipe_has_crobat()) {
      run_turn_issue_1751_original();
      return;
    }

    run_turn_issue_1751_core();
    if (complete_issue_1673_secret_box_payload_deadline()) {
      trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
            "Completed the deadline Secret Box payload route: " + state_line());
    }
    complete_issue_1724_crobat_stadium_compression();
    if (complete_issue_1674_turo_late_promotion()) {
      trace("POLICY", "P-AXIS-01; R-GAME-SUPPORTER",
            "Completed the deadline Professor Turo promotion route: " + state_line());
    }
    if (complete_issue_1675_latias_late_promotion()) {
      trace("POLICY", "P-AXIS-01; R-GAME-BENCH; R-GAME-RETREAT",
            "Completed the deadline Latias ex promotion route: " + state_line());
    }
  }

  bool complete_issue_1724_crobat_stadium_compression() {
    // Public K1 begins after either a legal deck inspection or a complete legal
    // Prize inspection; Hisuian Heavy Ball reveals every face-down Prize card:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // https://github.com/FlareZ123/pokemon-sims/issues/1994
    const bool payload_remains_in_known_deck =
        prizes_known() && payload_might_be_in_deck();
    const bool direct_burnet_draw = !state_.supporter_used && supporter_allowed() &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        payload_remains_in_known_deck;
    const bool direct_blender_draw = !item_locked() &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
        payload_remains_in_known_deck;

    // The wrapped turn policy has already evaluated its deterministic Item,
    // Supporter, evolution, attachment, VSTAR Power, retreat, and attack routes.
    // This final retry is legal only from the exact visible seven-card state where
    // playing Chaotic Swell and then Crobat V changes Dark Asset from zero draws to
    // one draw. K1 must prove at least one direct current-turn payload outlet remains
    // in the deck. Selection never reads the next card's identity:
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official Stadium, Bench, Ability, and action-order procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Deterministic-route priority and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Future-card-oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1724
    // Prize-derived K1 regression: https://github.com/FlareZ123/pokemon-sims/issues/1994
    if (!prizes_known() || scenario_.locks != LockMode::None || state_.turn_ended ||
        !setup_axis_missing() || state_.stadium_used ||
        state_.stadium != Stadium::None ||
        hand_count(Card::ChaoticSwell) == 0 ||
        hand_count(Card::CrobatV) == 0 || state_.hand.size() != 7U ||
        state_.dark_asset_used || bench_space() <= 0 || state_.deck.empty() ||
        !ability_available_for_pokemon(Card::CrobatV) ||
        forretress_combo_enabled() ||
        (!direct_burnet_draw && !direct_blender_draw) ||
        (hand_count(Card::Oricorio) > 0 && needs_oricorio_connector()) ||
        (hand_count(Card::TapuLeleGX) > 0 && needs_tapu_connector())) {
      return false;
    }

    remove_one(state_.hand, Card::ChaoticSwell);
    state_.stadium_used = true;
    state_.stadium = Stadium::ChaoticSwell;
    state_.path_lock_removed = true;
    trace("PLAY STADIUM", "R-CHAOTIC-SWELL-01; R-CROBAT-01; P-COMPRESS-01",
          "Played Chaotic Swell into the empty Stadium zone to make the legal Crobat V Dark Asset draw live.");

    // The current issue-1724 guard above has already checked every connector-priority
    // condition that decides whether Crobat is the selected Basic. Re-entering
    // bench_crobat_if_useful() would run an older macro-bound Tapu/Oricorio policy
    // and can contradict that completed decision. After Chaotic Swell resolves,
    // Crobat V is still a Basic in hand and the proven Bench slot remains available,
    // so execute the selected legal hand-to-Bench transition directly:
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Official Basic Bench and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Current-route priority contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed stale-recheck bug: https://github.com/FlareZ123/pokemon-sims/issues/2252
    if (!bench_from_hand(Card::CrobatV, true)) {
      throw std::logic_error(
          "Issue-1724 Crobat V Bench action disappeared after legal compression");
    }

    // Dark Asset may expose a current-turn Item, Supporter, Basic, VSTAR card,
    // Energy, or promotion route. Reopen the same legal fixed points before the
    // ready-state check, while every existing once-per-turn and lock guard remains
    // authoritative:
    // Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Core Item, Supporter, Bench, evolution, attachment, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1724
    play_items_until_stable(true);
    attach_manual();
    evolve_best_regi_issue_1224();
    play_basics_from_hand();
    play_items_until_stable(true);
    evolve_best_regi_issue_1224();
    if (!state_.supporter_used && setup_axis_missing()) {
      choose_supporter_issue_1152();
    }
    if (use_legacy_star_issue_1152()) {
      play_basics_from_hand();
      play_items_until_stable(true);
      if (!state_.supporter_used && setup_axis_missing()) {
        choose_supporter_issue_1152();
        play_items_until_stable(true);
      }
    }
    resolve_final_promotion_and_attachment();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    attach_powerglass();
    trace("POLICY", "P-AXIS-01; R-CROBAT-01; R-CHAOTIC-SWELL-01",
          "Completed the late Crobat V Stadium-compression retry: " + state_line());
    return true;
  }

#include "core/routes/issue_1673_secret_box_payload_deadline_policy.inc"

  void run_turn_issue1674_original() {
    run_turn_issue1673_original();
    if (complete_issue_1673_secret_box_payload_deadline()) {
      trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
            "Completed the deadline Secret Box payload route: " + state_line());
    }
    complete_issue_1724_crobat_stadium_compression();
  }

  bool issue_1674_turo_promotion_ready() {
    return !state_.turn_ended && state_.turn == scenario_.max_turn &&
        need_active_vstar() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_payload() &&
        hand_count(Card::ProfessorTuro) > 0 && benched_vstar_promotion_ready();
  }

  bool complete_issue_1674_turo_late_promotion() {
    if (!issue_1674_turo_promotion_ready()) {
      return false;
    }

    // The Benched Regidrago VSTAR already has GGF, and Earthen Vessel paid
    // Dragapult ex as the strict-JIT payload this turn. Professor Turo's Scenario
    // returns the Basic Active Pokémon to hand, which immediately requires a
    // Benched replacement; promoting the complete VSTAR resolves the sole remaining
    // Active-position axis without another discard, attachment, or search:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter and Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and Supporter contention: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1674
    return play_turo_active_promotion_route();
  }

  void run_turn_issue1675_original() {
    run_turn_issue1674_original();
    if (complete_issue_1674_turo_late_promotion()) {
      trace("POLICY", "P-AXIS-01; R-GAME-SUPPORTER",
            "Completed the deadline Professor Turo promotion route: " + state_line());
    }
  }

#include "part_issue_1675_latias_late_promotion_override.inc"
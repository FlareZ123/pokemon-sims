  bool issue_1751_crobat_pending() {
    // A held Crobat remains a live probabilistic connector while a setup axis is
    // missing. Deterministic Tapu Lele-GX and Oricorio connectors retain priority:
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Observable-state route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
    return hand_count(Card::CrobatV) > 0 && !state_.dark_asset_used &&
        bench_space() > 0 && !state_.deck.empty() && setup_axis_missing() &&
        ability_available_for_pokemon(Card::CrobatV) &&
        !(hand_count(Card::Oricorio) > 0 && needs_oricorio_connector()) &&
        !(hand_count(Card::TapuLeleGX) > 0 && needs_tapu_connector());
  }

  bool issue_1751_use_fss_if_no_pending_crobat() {
    // Attaching Forest Seal Stone compresses the hand, while Star Alchemy adds a
    // searched card. Preserve the Power until after Dark Asset unless no Crobat
    // window remains, matching the existing issue-1726 compression contract:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    // https://github.com/FlareZ123/pokemon-sims/issues/1726
    // https://github.com/FlareZ123/pokemon-sims/issues/1751
    if (issue_1751_crobat_pending()) return false;
    return use_fss();
  }

  bool issue_1751_resolve_crobat_after_compression(
      const bool permit_payload) {
    issue_1751_defer_crobat_ = false;
    if (!bench_crobat_if_useful()) return false;

    // Dark Asset can expose every remaining current-turn action class. Reopen the
    // fixed points after the draw while preserving once-per-turn, lock, DCI, K0/K1,
    // and deterministic-route guards in their authoritative implementations:
    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Core turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Future-card-oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
    issue_1751_defer_crobat_ = true;
    play_items_until_stable(permit_payload);
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(permit_payload);
    attach_manual();
    evolve_best_regi();
    play_basics_from_hand();
    play_items_until_stable(true);
    evolve_best_regi();
    issue_1751_defer_crobat_ = false;
    return true;
  }

  void run_turn_issue_1910_late_arven_blender_replay() {
    const int blender_before = hand_count(Card::BrilliantBlender);
    const int arven_discard_before = count_of(state_.discard, Card::Arven);
    const bool supporter_before = state_.supporter_used;
    run_turn_issue_1751_original();

    const bool arven_exposed_live_blender =
        !supporter_before && state_.supporter_used && !state_.turn_ended &&
        count_of(state_.discard, Card::Arven) > arven_discard_before &&
        blender_before == 0 && hand_count(Card::BrilliantBlender) > 0 &&
        !item_locked() && deck_seen_ && !need_regi() && !need_vstar() &&
        !need_energy() && !need_active_vstar() && need_payload() &&
        can_play_payload_this_turn() && payload_might_be_in_deck();
    if (!arven_exposed_live_blender) return;

    // Arven searches an Item without ending the turn, so a newly exposed
    // Brilliant Blender remains legal in the same action phase. Once K1 proves
    // a Dragon payload remains in deck and every non-payload setup axis is
    // complete, play the searched ACE SPEC before the enclosing ready-state check:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, search, discard, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1910
    if (!play_brilliant_blender()) {
      throw std::logic_error(
          "Proven issue-1910 Arven-Blender continuation became unavailable");
    }
  }

// The registered shell path enters the final issue-1751 wrapper after this include.
// Route that call through the narrow post-Arven Item replay while preserving the
// original turn implementation for every other state:
// https://github.com/FlareZ123/pokemon-sims/issues/1910
#define run_turn_issue_1751_original run_turn_issue_1910_late_arven_blender_replay

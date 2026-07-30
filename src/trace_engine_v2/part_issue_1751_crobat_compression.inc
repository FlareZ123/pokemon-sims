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

      if (bench_oricorio_if_useful()) return true;
      if (play_pokemon_communication(permit_payload)) return true;
      #include "part_014a_issue_962_eligibility.inc"
      #include "part_014a_issue_962_projection.inc"
      #include "part_014a_issue_962_decision.inc"

      // Prefer the higher-connectivity Quick Ball route only when cost-aware
      // projections prove Tapu Lele-GX -> Wonder Tag -> Crispin preserves both
      // paid Items while Earthen Vessel-first exhausts Quick Ball's legal cost:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/962
      if (quick_ball_tapu_crispin_route_should_precede_vessel() &&
          play_quick_ball(permit_payload)) return true;
      if (play_earthen_vessel(permit_payload)) return true;
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
    }
    if (permit_payload && need_payload()) {
      if (play_brilliant_blender()) return true;
      // Evolution Incense can fetch a modeled Evolution Dragon for a separately
      // held discard Item in this same ready turn: https://api.pokemontcg.io/v2/cards/swsh1-163
      if (play_evolution_incense(permit_payload)) return true;
      if (play_pokemon_communication(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_quick_ball(permit_payload)) return true;
      // Ultra Ball's discard cost can put a current-turn Dragon payload in discard: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      if (play_ultra_ball(permit_payload)) return true;
      if (play_earthen_vessel(permit_payload)) return true;
    }
    return false;
  }

  void play_items_until_stable(const bool permit_payload) {
    if (state_.turn_ended) return;
    for (int guard = 0; guard < 32; ++guard) {
      // Pineco may be Benched, legally evolve on a later turn, and use Exploding
      // Energy through Item lock once a Rule Box Ability lock is absent or removed.
      // Search Items remain blocked when Item lock applies:
      // https://api.pokemontcg.io/v2/cards/sv4pt5-1
      // https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/issues/972
      if (advance_forretress_combo()) continue;
      if (item_locked()) break;
      // Hisuian Heavy Ball is optional, so the policy must retain it once no setup
      // axis remains instead of spending a non-advancing Prize-information action: https://api.pokemontcg.io/v2/cards/swsh10-146
      if (!setup_axis_missing()) break;
      if (run_forretress_combo_search_step(permit_payload)) continue;
      if (play_heavy_ball()) continue;
      if (!run_search_items_one_step(permit_payload)) break;
    }
  }

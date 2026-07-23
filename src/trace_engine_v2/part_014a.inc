      if (bench_oricorio_if_useful()) return true;
      if (play_pokemon_communication(permit_payload)) return true;
      #include "part_014a_issue_962_eligibility.inc"
      #include "part_014a_issue_962_projection.inc"
      #include "part_014a_issue_962_decision.inc"
      #include "part_014a_issue_1447_vessel_hold.inc"

      // Prefer the higher-connectivity Quick Ball route only when cost-aware
      // projections prove Tapu Lele-GX -> Wonder Tag -> Crispin preserves both
      // paid Items while Earthen Vessel-first exhausts Quick Ball's legal cost:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sm2-60
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/962
      if (quick_ball_tapu_crispin_route_should_precede_vessel() &&
          play_quick_ball(permit_payload)) return true;
      if (issue_1447_should_hold_vessel()) return false;
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
      // A complete same-turn Forest Seal Stone route makes another Pineco a
      // future-only connector. Preserve both Quick Balls and the Bench slot:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4pt5-1
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/1412
      const auto issue_1412_direct_route_visible = [this] {
        const Pokemon* regi = state_.active ? &*state_.active : nullptr;
        const bool active_is_regi = regi != nullptr &&
            (regi->card == Card::RegidragoV ||
             regi->card == Card::RegidragoVstar);
        const bool evolution_complete = active_is_regi &&
            (regi->card == Card::RegidragoVstar ||
             (regi->card == Card::RegidragoV &&
              regi->entered_turn < state_.turn &&
              hand_count(Card::RegidragoVstar) > 0));
        const auto can_hold_fss = [](const Pokemon& pokemon) {
          return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
        };
        const bool fss_attachable =
            hand_count(Card::ForestSealStone) > 0 &&
            !state_.vstar_power_used &&
            ((state_.active && can_hold_fss(*state_.active)) ||
             std::any_of(state_.bench.begin(), state_.bench.end(),
                         can_hold_fss));
        const bool fire_available = hand_count(Card::Fire) > 0 ||
            (might_be_unseen(Card::Fire) &&
             (secret_box_has_live_fss_holder() || fss_attachable));
        return outcome_.used_exploding_energy && secret_box_combo_enabled() &&
            !state_.turn_ended && !rule_box_abilities_locked() &&
            !state_.manual_energy_used && regi != nullptr &&
            regi->grass >= 2 && regi->fire == 0 && payload_ready() &&
            evolution_complete && fire_available;
      };
      if (!issue_1412_direct_route_visible() &&
          run_forretress_combo_search_step(permit_payload)) continue;
      if (play_heavy_ball()) continue;
      if (!run_search_items_one_step(permit_payload)) break;
    }
  }

#include "part_issue_1798_steven_blender_route.inc"

  bool complete_issue_1675_latias_late_promotion() {
    if (state_.turn_ended || state_.turn != scenario_.max_turn ||
        !need_active_vstar() || need_regi() || need_vstar() ||
        need_energy() || need_payload() || state_.retreat_used ||
        bench_space() == 0 || hand_count(Card::LatiasEx) == 0 ||
        in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        !benched_vstar_promotion_ready()) {
      return false;
    }

    // The Benched Regidrago VSTAR already has GGF, and Mysterious Treasure paid
    // Dialga-GX as this turn's strict-JIT payload. Latias ex is held with open Bench
    // space. Benching it makes Skyliner live, which gives the Basic Active no Retreat
    // Cost and promotes the complete VSTAR without consuming the held Supporter:
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dialga-GX Dragon payload: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Bench, Ability, retreat, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest resource-preserving route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1675
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("Issue-1675 Latias ex became unbenchable");
    }
    // The shared helper enforces the once-per-turn retreat and zero-cost Skyliner
    // gates before exchanging the Basic Active for the prepared VSTAR:
    // https://api.pokemontcg.io/v2/cards/sv8-76 https://www.pokemon.com/us/pokemon-tcg/rules
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("Issue-1675 Skyliner promotion became unavailable");
    }
    return true;
  }

  void run_turn_issue1775_original() {
    if (complete_issue_1796_t3_finish()) {
      // The K1 Wonder Tag route has completed every public setup axis on T3:
      // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1796
      trace("POLICY", "P-AXIS-01; R-STEVEN-01; R-MT-01; P-JIT-01; P-COMPRESS-01",
            "Completed the Wonder Tag, Steven, Treasure, and Latias ex T3 route: " +
                state_line());
      return;
    }
    // A route committed by the prior two public Supporter decisions must finish before
    // generic Item and discard selection can spend Earthen Vessel or its Dragon cost:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1795
    if (complete_issue_1795_vessel_finish()) {
      trace("POLICY", "P-AXIS-01; R-EV-01; P-JIT-01; P-COMPRESS-01",
            "Completed the committed Crispin, Steven, and Earthen Vessel T4 route: " +
                state_line());
      return;
    }

    // Finish the committed T4 route before generic actions can spend Brilliant
    // Blender, its deck payload, the searched VSTAR, or the searched Grass:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1798
    if (complete_issue_1798_steven_blender_route()) {
      trace("POLICY", "P-AXIS-01; R-STEVEN-01; R-BLENDER-01; P-JIT-01; P-COMPRESS-01",
            "Completed the committed Steven and Brilliant Blender T4 route: " +
                state_line());
      return;
    }

    // Commit the complete K1 route before ordinary attachment heuristics reject the
    // surplus-type Fire and before Celestial Roar introduces hidden-card variance:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1798
    if (start_issue_1798_steven_blender_route()) {
      trace("TURN END", "R-STEVEN-01; P-KNOWLEDGE-01; P-COMPRESS-01",
            state_line());
      return;
    }

    run_turn_issue1675_original();
    if (complete_issue_1675_latias_late_promotion()) {
      trace("POLICY", "P-AXIS-01; R-GAME-BENCH; R-GAME-RETREAT",
            "Completed the deadline Latias ex promotion route: " + state_line());
    }
  }

#include "part_issue_1775_gladion_treasure_route_override.inc"

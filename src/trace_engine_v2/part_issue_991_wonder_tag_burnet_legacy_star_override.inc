#define bench_tapu_if_useful bench_tapu_if_useful_issue_1513_original
#include "part_issue_991_wonder_tag_burnet_legacy_star_override_base.inc"
#undef bench_tapu_if_useful

  bool held_turo_direct_active_promotion_route() const {
    // Professor Turo's Scenario directly removes a Basic Active and forces a
    // Benched replacement. When a complete Regidrago VSTAR is already Benched,
    // preserve Tapu Lele-GX and its one-shot Wonder Tag instead of creating a
    // longer Tapu -> Tate & Liza route for the same Active-position axis:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter and Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Direct-connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1513
    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        state_.active->grass >= 2 && state_.active->fire >= 1;

    // Under Item lock, Turo may solve the Active axis one turn before Burnet solves
    // the strict-JIT payload axis. Preserve the existing delayed-Burnet contract:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Prior route contract: https://github.com/FlareZ123/pokemon-sims/issues/900
    const bool delayed_burnet_payload_route = strict_payload_timing() && item_locked() &&
        need_payload() && state_.turn < scenario_.max_turn &&
        hand_count(Card::ProfessorBurnet) > 0 && payload_might_be_in_deck();
    if (!supporter_allowed() || hand_count(Card::ProfessorTuro) == 0 || !state_.active ||
        !is_basic(state_.active->card) || active_regidrago_has_ready_direct_evolution ||
        !need_active_vstar() || need_regi() || need_vstar() || need_energy() ||
        (need_payload() && !delayed_burnet_payload_route) || can_free_retreat_with_latias() ||
        hand_count(Card::TateLiza) > 0) {
      return false;
    }

    return std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 && pokemon.fire >= 1;
    });
  }

  bool bench_tapu_if_useful() {
    if (held_turo_direct_active_promotion_route()) {
      // Held Professor Turo already removes the Basic Active and promotes the
      // complete Benched Regidrago VSTAR. Do not spend Tapu Lele-GX, its Bench
      // slot, Wonder Tag, and Tate & Liza on the same Active-position axis:
      // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
      // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Direct-connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1513
      trace_tapu_hold_once(
          "issue-1513-held-turo-active-promotion",
          "R-TURO-01; R-TAPU-01; R-TATE-01; R-GAME-SUPPORTER; P-CONNECTOR-01",
          "Retained Tapu because held Professor Turo directly promotes the complete Benched Regidrago VSTAR.");
      return false;
    }
    return bench_tapu_if_useful_issue_1513_original();
  }

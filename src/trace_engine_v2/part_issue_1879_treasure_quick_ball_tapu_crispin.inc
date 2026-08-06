  // Preserve the prior issue-1879 and issue-2199 implementation as a named
  // preprocessor layer, then replace only the gate that incorrectly inherited
  // the caller's generic payload permission. The issue-specific availability
  // proof independently requires exact K1, a current-turn JIT payload deficit,
  // Latias ex in the inspected deck, Skyliner, an open Bench, an unused Retreat,
  // and an already powered Benched Regidrago VSTAR:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Bench, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, current-turn JIT, DCI, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2199
#define play_quick_ball_issue_2199_dispatch \
  play_quick_ball_issue_2199_dispatch_generic_gate
#include "part_issue_1879_treasure_quick_ball_tapu_crispin_base.inc"
#undef play_quick_ball_issue_2199_dispatch

  bool play_quick_ball_issue_2199_dispatch(const bool permit_payload) {
    if (play_issue_2199_quick_ball_latias_payload_route()) return true;
    return play_quick_ball_issue_2199_dispatch_generic_gate(permit_payload);
  }

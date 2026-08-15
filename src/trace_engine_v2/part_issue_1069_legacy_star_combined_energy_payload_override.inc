// #3199 public-projection isolation wrapper. The implementation remains byte-for-byte
// in the adjacent canonical base include; this boundary only prevents Tate's held-route
// shadow simulation from resolving Legacy Star against hidden future deck order.
// Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
// Advanced procedure source: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
// Confirmed semantic bug: https://github.com/FlareZ123/pokemon-sims/issues/3199
#define use_legacy_star use_legacy_star_issue3199_public_projection_original
#include "core/routes/issue_3199_legacy_star_combined_energy_payload_base.inc"
#undef use_legacy_star

#include "core/routes/issue_3199_public_projection_state.inc" // Canonical state owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_3199_public_projection_state.inc

  bool use_legacy_star() {
    if (issue_3199_public_projection_depth_ > 0) return false;
    return use_legacy_star_issue3199_public_projection_original();
  }

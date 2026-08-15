// #3199 isolates only Legacy Star while Tate's public held-route classifier runs.
// Forest Seal Stone and every other legal deterministic route remain available to
// the projected Engine. A thread-local depth is used because the classifier creates
// a second Engine on the same worker thread; RAII restores the guard on exceptions.
// Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
// Confirmed semantic bug: https://github.com/FlareZ123/pokemon-sims/issues/3199
#define tate_draw_has_held_non_supporter_completion \
  tate_draw_has_held_non_supporter_completion_issue3199_original
#include "issue_3199_turo_oricorio_base.inc"
#undef tate_draw_has_held_non_supporter_completion

  struct PublicProjectionDepthGuard {
    int& depth;

    explicit PublicProjectionDepthGuard(int& value) : depth(value) { ++depth; }
    ~PublicProjectionDepthGuard() { --depth; }

    PublicProjectionDepthGuard(const PublicProjectionDepthGuard&) = delete;
    PublicProjectionDepthGuard& operator=(const PublicProjectionDepthGuard&) = delete;
  };

  bool tate_draw_has_held_non_supporter_completion() const {
    PublicProjectionDepthGuard guard(issue_3199_public_projection_depth_);
    return tate_draw_has_held_non_supporter_completion_issue3199_original();
  }

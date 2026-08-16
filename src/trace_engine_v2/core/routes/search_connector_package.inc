// Canonical search-route composition package. Keep the historical Heavy Ball then
// connector-helper order while giving the pair one named ownership boundary.
// Hisuian Heavy Ball owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/hisuian_heavy_ball_policy.inc
// Search connector helper owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/search_connector_helpers.inc
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
#if !defined(play_mysterious_treasure) || !defined(play_heavy_ball)
#error "Search-route package requires caller-owned Treasure and Heavy Ball aliases"
#endif

#include "hisuian_heavy_ball_policy.inc"
#include "search_connector_helpers.inc"

#if !defined(play_mysterious_treasure) || !defined(play_heavy_ball)
#error "Search-route package must preserve caller-owned search aliases"
#endif

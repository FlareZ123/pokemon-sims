// Late Steven/VSTAR Vessel composition owner. Keep this include at the historical
// boundary after part_010_late_steven_override.inc so the alias chain observes the
// same preceding members and releases play_brilliant_blender at the same point.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
// Canonical route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_vstar_vessel_route_policy.inc

  Card choose_supporter_after_search_started_issue3202_const_fallback() const {
    return choose_supporter_after_search_started();
  }
#define late_steven_vstar_vessel_route_available late_steven_vstar_vessel_route_available_issue3202_original
#define choose_supporter_after_search_started choose_supporter_after_search_started_issue3202_original
#define should_play_steven should_play_steven_issue3202_original
#include "../part_010_steven_crispin_override.inc"
#undef should_play_steven
#undef choose_supporter_after_search_started
#undef late_steven_vstar_vessel_route_available
#include "../core/routes/steven_vstar_vessel_route_policy.inc"
#undef play_brilliant_blender

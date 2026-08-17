// Canonical Steven/Blender composition owner. Keep this block at the historical
// boundary immediately after part_009b2.inc, where play_ultra_ball is intentionally
// live. Local aliases are established and released here so the parent composition
// only preserves the ordering contract.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Steven/Blender source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010.inc
// Legacy Supporter cards: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/sm7-145
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136

#if defined(play_evolution_incense) || defined(play_earthen_vessel) || \
    defined(play_brilliant_blender)
#error "Steven/Blender composition requires search aliases to be undefined on entry"
#endif

#define play_evolution_incense play_evolution_incense_original
#define play_earthen_vessel play_earthen_vessel_empty_deck_original
#define play_brilliant_blender play_brilliant_blender_legacy_original
#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#define should_play_steven should_play_steven_original
#include "../part_010.inc"
#undef should_play_steven
#undef attach_fss
#undef fss_target_after_search_started

// Canonical late-Steven package. The historical part_010 path remains as a
// source-contract mirror until its anchored documentation and tests migrate.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/late_steven/package.inc
#include "../core/routes/late_steven/package.inc"

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
// Canonical issue-3202 route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_vstar_vessel_route_policy.inc
#include "../core/routes/steven_vstar_vessel_route_policy.inc"
#undef play_brilliant_blender

// The thinning policy remains the implementation wrapped by the later empty-deck
// guard, while the legacy part_010 implementation stays dormant:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010_blender_thinning_override.inc#L1-L77
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_empty_deck_search_override.inc#L78-L85
#define play_brilliant_blender play_brilliant_blender_empty_deck_original
#include "../part_010_blender_thinning_override.inc"
#undef play_brilliant_blender
#undef play_earthen_vessel
#undef play_evolution_incense
#undef play_ultra_ball

#if defined(play_evolution_incense) || defined(play_earthen_vessel) || \
    defined(play_brilliant_blender) || defined(play_ultra_ball) || \
    defined(fss_target_after_search_started) || defined(attach_fss) || \
    defined(should_play_steven) || defined(choose_supporter_after_search_started) || \
    defined(late_steven_vstar_vessel_route_available)
#error "Steven/Blender composition leaked a local wrapper alias"
#endif

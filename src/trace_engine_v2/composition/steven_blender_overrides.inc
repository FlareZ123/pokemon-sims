// Steven/Blender composition owner. Keep this block at the historical boundary
// immediately after part_009b2.inc, where play_ultra_ball is intentionally live.
// The local aliases are established and released through named composition owners
// so this parent only preserves their behavior-significant ordering contract.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Steven/Blender source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010.inc
// Legacy Supporter cards: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/sm7-145
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136

#if defined(play_evolution_incense) || defined(play_earthen_vessel) || \
    defined(play_brilliant_blender)
#error "Steven/Blender composition requires search aliases to be undefined on entry"
#endif

#include "steven_search_overrides.inc"
#include "steven_vstar_vessel_overrides.inc"
#include "blender_thinning_overrides.inc"

#if defined(play_evolution_incense) || defined(play_earthen_vessel) || \
    defined(play_brilliant_blender) || defined(play_ultra_ball) || \
    defined(fss_target_after_search_started) || defined(attach_fss) || \
    defined(should_play_steven) || defined(choose_supporter_after_search_started) || \
    defined(late_steven_vstar_vessel_route_available)
#error "Steven/Blender composition leaked a local wrapper alias"
#endif

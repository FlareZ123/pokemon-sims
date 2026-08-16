// Tapu Lele-GX search and Forest Seal Stone composition owner.
// Keep this complete wrapper family at the historical post-Steven boundary. Every
// alias established here is released before returning to opening_engine_overrides.inc.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#composition-ownership

#ifdef bench_tapu_if_useful
#error "Tapu search composition requires bench_tapu_if_useful to be undefined on entry"
#endif

#define bench_tapu_if_useful bench_tapu_if_useful_empty_deck_original
#define wonder_tag_crispin_is_redundant_with_held_complete_route \
  wonder_tag_crispin_is_redundant_with_held_complete_route_issue980_original
#define needs_tapu_connector needs_tapu_connector_issue989_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_tapu_tate_switch_override.inc"
#undef in_play
#undef needs_tapu_connector
#undef wonder_tag_crispin_is_redundant_with_held_complete_route
#undef bench_tapu_if_useful

// Canonical Wonder Tag route policy is composed directly from core/routes/.
// Tapu Lele-GX / Wonder Tag card data: https://api.pokemontcg.io/v2/cards/sm2-60
// Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/tapu_wonder_tag_route_policy.inc
// Confirmed route ownership: https://github.com/FlareZ123/pokemon-sims/issues/2270
#include "../core/routes/tapu_wonder_tag_route_policy.inc"

#define play_ultra_ball play_ultra_ball_empty_deck_original
#define play_evolution_incense play_evolution_incense_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_search_item_overrides.inc"
#undef in_play
#undef play_evolution_incense
#undef play_ultra_ball

#include "../part_pokemon_communication.inc"

#if defined(fss_target_after_search_started)
#error "Tapu/FSS composition requires its FSS wrapper alias to be undefined on entry"
#endif

#define fss_target_after_search_started fss_target_after_search_started_issue1071_original
#include "../part_010_fss_override.inc"
#undef fss_target_after_search_started
#define fss_target_after_search_started fss_target_after_search_started_issue1735_original
#include "../part_issue_1071_fss_oricorio_treasure_decomposition_override.inc"
#undef fss_target_after_search_started
#include "../part_issue_1735_fss_blender_priority_override.inc"
#include "../part_010_attach_fss_override.inc"

#if defined(fss_target_after_search_started)
#error "Tapu/FSS composition leaked its FSS wrapper alias"
#endif
#ifdef bench_tapu_if_useful
#error "Tapu search composition leaked bench_tapu_if_useful"
#endif

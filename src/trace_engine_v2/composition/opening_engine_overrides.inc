#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// Deck visibility policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_visibility_policy.inc
#include "../core/deck_visibility_policy.inc"
#undef begin_turn
// Turn lifecycle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/turn_lifecycle.inc
#include "../core/turn_lifecycle.inc"
// Tapu connector policy now owns its adjacent historical aliases as one core unit.
#include "../core/tapu_connector_policy.inc"
#include "../core/payload_hand_policy.inc"

// Opening Bench/Tate alias stack is centralized so its macro lifetime is audited
// in one composition unit.
#include "opening_bench_tate_stack.inc"

// Keep post-evolution discard provenance, Tate handling, and the following Item
// search wrapper family in the same ordered composition spine.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#include "../part_discard_recovery_provenance_override.inc"
#include "../part_tate_blender_attachment_override.inc"
#include "../part_tate_blender_tate_override.inc"

// Search-item wrapper aliases are centralized while preserving the intentional
// play_ultra_ball export into the Steven/Blender layer.
#include "opening_search_item_stack.inc"

// Continue into the Steven/Blender, legacy Supporter, Supporter selector, and
// VSTAR stages in historical order. Each composed boundary owns its local aliases;
// use_fss and use_celestial_roar intentionally remain live for the post-part_014a
// continuation.
// Steven/Blender source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010.inc
// Legacy Supporter cards: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/sm7-145
// Supporter selector cards: https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/swsh12-164
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136

// Centralize the core search, Steven, Forest Seal Stone, Brilliant Blender,
// Tapu connector, late search-item, and FSS target wrapper stack. Macro
// entry/exit guards make this composition boundary explicit.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

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

#include "../part_010_late_steven_override.inc"

#define should_play_steven should_play_steven_issue1030_original
#include "../part_010_steven_crispin_override.inc"
#undef should_play_steven
#undef play_brilliant_blender

// Isolate the active Brilliant Blender thinning layer and release the search-item
// aliases at the same legacy boundary used by this Steven composition.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
//
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
    defined(should_play_steven)
#error "Steven/Blender composition leaked a local wrapper alias"
#endif

// Continue into the Tapu connector, late search-item wrappers, and Forest Seal
// Stone target stack. Wrapper order remains behavior-significant because each
// layer composes with the implementation exposed by the preceding layer.
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
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

#include "../part_issue_989_wonder_tag_complete_route_override.inc"

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

// Supporter and VSTAR tail aliases are centralized; the two documented exports
// remain live for post_014a_overrides.inc.
#include "opening_supporter_vstar_stack.inc"

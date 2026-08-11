#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "../part_begin_turn_override.inc"
// Tapu connector policy now owns its adjacent historical aliases as one core unit.
#include "../core/tapu_connector_policy.inc"
#include "../core/payload_hand_policy.inc"

// Preserve the legacy opening Bench, connector, recovery, discard, and Tate aliases
// in one composition spine. choose_discard flows from part_006 through the
// part_007/008a continuation before the issue-1436/2323 policy handoff replaces it.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical split sources:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_006.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_007.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_008a.inc

#if defined(bench_from_hand) || defined(bench_oricorio_if_useful) || \
    defined(needs_oricorio_connector) || defined(needs_tapu_connector) || \
    defined(bench_tapu_if_useful) || defined(recover_discard_to_hand) || \
    defined(choose_discard)
#error "Opening bench-alias composition requires aliases to be undefined on entry"
#endif

#define bench_from_hand bench_from_hand_empty_deck_original
#define bench_oricorio_if_useful bench_oricorio_if_useful_empty_deck_original
#define needs_oricorio_connector needs_oricorio_connector_original
#define needs_tapu_connector needs_tapu_connector_original
#define bench_tapu_if_useful bench_tapu_if_useful_original
#define recover_discard_to_hand recover_discard_to_hand_name_only_original
#define choose_discard choose_discard_issue1436_original
#include "../part_006.inc"
#undef recover_discard_to_hand
#undef bench_tapu_if_useful
#undef needs_tapu_connector
#undef needs_oricorio_connector
#undef bench_oricorio_if_useful
#undef bench_from_hand

#if defined(bench_from_hand) || defined(bench_oricorio_if_useful) || \
    defined(needs_oricorio_connector) || defined(needs_tapu_connector) || \
    defined(bench_tapu_if_useful) || defined(recover_discard_to_hand)
#error "Opening bench-alias composition leaked a local alias"
#endif

#if defined(attach_manual) || defined(play_tate_switch) || defined(play_tate_draw)
#error "Tate legacy composition requires local aliases to be undefined on entry"
#endif
#if !defined(choose_discard)
#error "Tate legacy composition requires the opening choose_discard alias"
#endif

#define attach_manual attach_manual_tate_blender_original
#include "../part_007.inc"
#undef attach_manual

#define play_tate_switch play_tate_switch_tate_blender_original
#define play_tate_draw play_tate_draw_tate_blender_original
#include "../part_008a.inc"
#undef play_tate_draw
#undef play_tate_switch

#undef choose_discard
#define choose_discard choose_discard_issue2323_original
#include "../part_issue_1436_full_supporter_dci_override.inc"
#undef choose_discard
#include "../part_issue_2323_final_payload_guard.inc"

#if defined(attach_manual) || defined(play_tate_switch) || \
    defined(play_tate_draw) || defined(choose_discard)
#error "Tate legacy composition leaked a local alias"
#endif

// Keep post-evolution discard provenance, Tate handling, and the following Item
// search wrapper family in the same ordered composition spine.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#include "../part_discard_recovery_provenance_override.inc"
#include "../part_tate_blender_attachment_override.inc"
#include "../part_tate_blender_tate_override.inc"

#if defined(play_mysterious_treasure) || defined(play_heavy_ball) || \
    defined(play_quick_ball) || defined(play_ultra_ball) || defined(in_play)
#error "Tate search-item composition requires wrapper aliases to be undefined on entry"
#endif

#define play_mysterious_treasure play_mysterious_treasure_empty_deck_original
#define play_heavy_ball play_heavy_ball_prize_payload_original
#include "../part_008b.inc"
#undef play_heavy_ball

// Wonder Tag is copy-specific. Preserve all existing selector controls while an
// in-play Tapu no longer suppresses a second physical copy:
// https://api.pokemontcg.io/v2/cards/sm2-60
// https://github.com/FlareZ123/pokemon-sims/issues/746
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009a.inc"
#undef in_play
#undef play_mysterious_treasure

#define play_quick_ball play_quick_ball_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009b1.inc"
#undef in_play
#undef play_quick_ball

#define play_ultra_ball play_ultra_ball_original
#include "../part_009b2.inc"

#if defined(play_mysterious_treasure) || defined(play_heavy_ball) || \
    defined(play_quick_ball) || defined(in_play)
#error "Tate search-item composition leaked a local wrapper alias"
#endif

// play_ultra_ball intentionally remains exported until the following Steven/Blender
// wrapper layer releases it at the same historical boundary.

// Continue into the Steven/Blender, legacy Supporter, Supporter selector, and
// VSTAR stages in historical order. Each composed boundary owns its local aliases;
// use_fss and use_celestial_roar intentionally remain live for the post-part_014a
// continuation.
// Steven/Blender: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc
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

// Preserve the Forest Seal Stone alias for the later post-part_014a wrapper chain.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
#define use_fss use_fss_latias_original
#define play_crispin play_crispin_empty_deck_original
#define play_professor_burnet play_professor_burnet_legacy_original
#define play_steven play_steven_empty_deck_original
#include "../part_011.inc"
#undef play_steven
#undef play_professor_burnet
#undef play_crispin
// use_fss intentionally remains defined for the post-part_014a FSS composition.
// Continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

// Keep the part_012 Supporter selector aliases at this composition boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
#define play_arven play_arven_original
#define play_gladion play_gladion_original
#define play_serena play_serena_issue1683_original
#include "../part_012.inc"
#undef play_serena
#undef play_gladion
#undef play_arven

// Preserve Celestial Roar for its later post-part_014a wrapper chain while
// keeping the part_013 VSTAR-power aliases in this parent composition stage.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
#define use_celestial_roar use_celestial_roar_original
#define use_legacy_star use_legacy_star_original
#include "../part_013.inc"
#undef use_legacy_star
// use_celestial_roar intentionally remains defined for the later composition.
// Continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

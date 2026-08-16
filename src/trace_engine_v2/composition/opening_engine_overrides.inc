#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// K0/K1 deck visibility and deck-knowledge policy live in the canonical core unit:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_knowledge.inc
#include "../core/deck_knowledge.inc"
#undef begin_turn
// Turn lifecycle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/turn_lifecycle.inc
#include "../core/turn_lifecycle.inc"
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

// Keep the historical base compiled for regression archaeology, but expose the
// card-class-backed base under the legacy wrapper name consumed by later layers.
#define play_quick_ball play_quick_ball_card_class_legacy_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009b1.inc"
#undef in_play
#undef play_quick_ball

// part_009b2.inc now owns the one live Quick Ball card-class composition boundary.
// Keeping that include centralized avoids a second guarded include at this layer.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Quick Ball boundary repair: https://github.com/FlareZ123/pokemon-sims/issues/3341
#define play_ultra_ball play_ultra_ball_original
#include "../part_009b2.inc"

#if defined(play_mysterious_treasure) || defined(play_heavy_ball) || \
    defined(play_quick_ball) || defined(in_play)
#error "Tate search-item composition leaked a local wrapper alias"
#endif

// play_ultra_ball intentionally remains exported until the following inline
// Steven/Blender layer releases it at the same historical boundary.
// Named sub-owner preserves that exact alias boundary while shrinking this spine.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc
#include "steven_blender_overrides.inc"

// The Tapu connector, late search-item wrappers, and Forest Seal Stone target stack
// have one complete local alias lifetime, so compose them through their named owner.
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Canonical composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/tapu_fss_overrides.inc
#include "tapu_fss_overrides.inc"

// The part_011/012/013 legacy Supporter selectors and VSTAR-power macro bodies now
// live directly in this sole early-composition owner. Preserve the historical
// textual order and the two continuation aliases consumed after part_014a.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
#if defined(use_fss) || defined(play_crispin) || \
    defined(play_professor_burnet) || defined(play_steven) || \
    defined(play_arven) || defined(play_gladion) || defined(play_serena) || \
    defined(use_celestial_roar) || defined(use_legacy_star)
#error "Legacy Supporter composition requires owned aliases to be undefined on entry"
#endif

#define use_fss use_fss_latias_original
#define play_crispin play_crispin_empty_deck_original
#define play_professor_burnet play_professor_burnet_legacy_original
#define play_steven play_steven_empty_deck_original
#include "../part_011.inc"
#include "../core/routes/steven_free_slot_connector_policy.inc"
#undef play_steven
#undef play_professor_burnet
#undef play_crispin

#if defined(play_crispin) || defined(play_professor_burnet) || defined(play_steven)
#error "Legacy Supporter composition leaked a local alias"
#endif

// use_fss intentionally crosses the part_011 boundary. post_014a_overrides.inc
// consumes and replaces this alias after part_014a.inc reaches the required member
// boundary. The final continuation contract below verifies that export together
// with the later Celestial Roar export.
// Continuation owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

// The part_012 selector follows part_011 immediately in the historical composition.
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
#if defined(play_arven) || defined(play_gladion) || defined(play_serena)
#error "Supporter selector composition requires aliases to be undefined on entry"
#endif

#define play_arven play_arven_original
#define play_gladion play_gladion_original
#define play_serena play_serena_issue1683_original
#include "../part_012.inc"
#undef play_serena
#undef play_gladion
#undef play_arven

#if defined(play_arven) || defined(play_gladion) || defined(play_serena)
#error "Supporter selector composition leaked a local alias"
#endif

// The VSTAR-power aliases follow the selector immediately. Compose the canonical
// named legacy runtime at this exact member boundary so no root compatibility
// forwarder is needed. Preserve the Celestial Roar alias for the later wrapper chain.
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Canonical runtime owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/supporter_legacy_runtime.inc
#if defined(use_celestial_roar) || defined(use_legacy_star)
#error "VSTAR-power composition requires aliases to be undefined on entry"
#endif

#define use_celestial_roar use_celestial_roar_original
#define use_legacy_star use_legacy_star_original
#include "../core/supporter_legacy_runtime.inc"
#undef use_legacy_star

#if defined(use_legacy_star)
#error "VSTAR-power composition leaked use_legacy_star"
#endif

// Both continuation aliases intentionally cross this composition boundary. Keep one
// exit contract beside their shared continuation owner so future reorganizations
// cannot silently sever either wrapper chain.
// C++ conditional preprocessing: https://eel.is/c++draft/cpp.cond
#if !defined(use_fss) || !defined(use_celestial_roar)
#error "Legacy Supporter composition must export use_fss and use_celestial_roar"
#endif

// post_014a_overrides.inc consumes and replaces both aliases after the part_014a
// boundary. Continuation owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
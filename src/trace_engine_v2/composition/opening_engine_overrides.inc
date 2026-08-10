#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "../part_begin_turn_override.inc"
#include "../core/tapu_connector_policy.inc"
#include "../core/payload_hand_policy.inc"
#include "../core/tapu_legacy_aliases.inc"

// part_006 owns the opening Bench/connector aliases and exports choose_discard for
// the Tate continuation below. Keep the boundary directly before part_007/008a.
// Composition source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_bench_body.inc
#include "opening_legacy_bench_body.inc"

// Keep the legacy Tate attachment/draw body in this same opening boundary. The
// exported choose_discard alias from part_006.inc is consumed by part_007.inc and
// part_008a.inc before the later discard-policy overrides replace it.
// Historical split sources:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_007.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_008a.inc
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

// part_007.inc opens evolve_best_regi(), and part_008a.inc completes it before
// the later Tate/discard member overrides continue in this composition spine.
// Keep these stages in their historical root order so exported wrapper aliases
// retain the same lifetime across the textual include boundary.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Tate/discard continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/tate_discard_overrides.inc
// Steven/Blender continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc
#include "tate_discard_overrides.inc"
#include "steven_blender_overrides.inc"

// Continue through the legacy Supporter, Supporter selector, and VSTAR stages in
// the same order formerly listed at the translation-unit root. These wrappers own
// the alias setup/teardown for their historical part files, while use_fss and
// use_celestial_roar intentionally remain live for the post-part_014a chain.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Legacy Supporter stage: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/legacy_supporter_body.inc
// Supporter selector stage: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/supporter_selector_body.inc
// VSTAR stage: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/vstar_power_body.inc
#include "legacy_supporter_body.inc"
#include "supporter_selector_body.inc"
#include "vstar_power_body.inc"

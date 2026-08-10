#pragma once

// Keep the legacy Tate attachment/draw body and its discard guard in one ordered
// composition boundary. The exported choose_discard alias from the opening layer
// is intentionally consumed by part_007.inc and part_008a.inc before being reset.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

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

// part_007.inc opens evolve_best_regi(), and part_008a.inc completes it before
// the later Tate/discard member overrides are included by the root translation unit:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_007.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_008a.inc

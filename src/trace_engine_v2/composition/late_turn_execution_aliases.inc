// Centralized late-turn execution aliases for the textual Engine composition.
// Keep these macros scoped across part_014c through part_016 exactly as the
// composition spine requires; the paired cleanup fragment releases them.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count)
#error "Late-turn composition requires execution aliases to be undefined on entry"
#endif
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#define hand_count hand_count_issue_3057

// Centralized setup for the late execution aliases consumed by part_014c through
// part_016. Keep this textual include immediately before part_014c.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable)
#error "Late-turn composition requires execution aliases to be undefined on entry"
#endif
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#define hand_count hand_count_issue_3057
#define play_items_until_stable play_items_until_stable_issue_3057

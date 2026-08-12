// Centralized teardown for the late execution aliases after part_016 closes the
// historical execution/reporting chain. Keep the leak guard beside the undefs.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#undef run_turn
#undef play_field_blower
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable)
#error "Late-turn composition leaked an execution alias"
#endif

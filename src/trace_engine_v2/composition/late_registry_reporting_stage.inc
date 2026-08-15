// Continue the historical scenario registry and reporting chain after part_014c.
// play_field_blower and run_turn arrive live from late_execution_stage.inc and are
// intentionally released only after part_016.inc closes the legacy chain.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical registry continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

// part_014c defines the historical scenario registry and leaves only the final
// return/brace of scenario_by_label() for the first two lines of part_015. The
// Garbodor wrapper is composed at namespace scope through part_forretress_ex_combo.inc.
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition requires aliases to be undefined on entry"
#endif
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for part_016.inc.
#include "../part_016.inc"

#undef scenario_by_label
#undef all_scenarios
#undef run_turn
#undef play_field_blower

#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable) || defined(all_scenarios) || \
    defined(scenario_by_label)
#error "Canonical engine composition leaked a late alias"
#endif

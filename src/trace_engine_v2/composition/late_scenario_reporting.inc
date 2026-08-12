// Centralized scenario-registry and final reporting composition. part_014c leaves
// the historical registry open for the first two lines of part_015; part_016 then
// closes the execution/reporting chain while the execution aliases remain live.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition requires aliases to be undefined on entry"
#endif
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for part_016.inc.
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
#include "../part_016.inc"

#undef scenario_by_label
#undef all_scenarios
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition leaked an alias"
#endif

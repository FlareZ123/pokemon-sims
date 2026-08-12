// Ordered late turn-execution, scenario-registry, and reporting composition.
// Keep these aliases live through part_016 exactly as the historical root wrapper
// did so the translation unit remains behaviorally identical.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#include "late_execution_alias_setup.inc"
#include "../part_014c.inc"
#undef play_items_until_stable
#undef hand_count
#undef ability_available_for_pokemon
#ifdef ability_available_for_pokemon
#error "Garbodor composition leaked ability_available_for_pokemon"
#endif

#include "late_scenario_reporting.inc"

// Release the late-turn textual-composition aliases after part_016 closes the
// historical execution/reporting chain. Keep the leak guards beside the undefs.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#undef run_turn
#undef play_field_blower
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable)
#error "Late-turn composition leaked an execution alias"
#endif

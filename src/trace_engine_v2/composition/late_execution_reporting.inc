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
#include "late_execution_alias_cleanup.inc"

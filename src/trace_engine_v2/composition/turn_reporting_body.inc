#pragma once

// Ordered late turn-execution, scenario-registry, and reporting composition.
// Keep these aliases live through part_016 exactly as the historical root wrapper
// did so the translation unit remains behaviorally identical.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Late turn source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc

#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "../part_014c.inc"
#undef ability_available_for_pokemon

// part_014c defines the historical scenario registry and leaves only the final
// return/brace of scenario_by_label() for the first two lines of part_015. The
// Garbodor wrapper is composed at namespace scope through part_forretress_ex_combo.inc.
// Route reporting and the production CLI through the appended two rows while the
// historical public registry remains available to focused tests.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for part_016.inc.
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
#include "../part_016.inc"
#undef scenario_by_label
#undef all_scenarios

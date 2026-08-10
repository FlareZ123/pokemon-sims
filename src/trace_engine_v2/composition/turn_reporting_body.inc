#pragma once

// Ordered composition for the late turn-execution and reporting body. Keep the
// historical aliases live through part_016 exactly as the translation-unit root
// did before this structural extraction.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition root: https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp

#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
// part_014c defines the historical scenario registry, with scenario_by_label()
// intentionally continuing into the first two lines of part_015. Compose those
// definitions under private names so the namespace-level Garbodor wrapper can append
// its rows without changing the legacy split files or their historical row order:
// https://api.pokemontcg.io/v2/cards/xy9-57
// https://api.pokemontcg.io/v2/cards/swsh7-142
// https://github.com/FlareZ123/pokemon-sims/issues/2808
#define all_scenarios all_scenarios_without_garbodor
#define scenario_by_label scenario_by_label_without_garbodor
#include "../part_014c.inc"
#undef scenario_by_label
#undef all_scenarios
#undef ability_available_for_pokemon

// The opening continuation in part_015 only closes the historical lookup body;
// route its later reporting/self-test lookups and the final CLI through the two-row
// Garbodor extension. Keep these aliases live through part_016:
// https://github.com/FlareZ123/pokemon-sims/issues/2808
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for the final implementation stage in part_016.inc.

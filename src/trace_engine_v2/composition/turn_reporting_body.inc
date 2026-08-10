#pragma once

// Ordered composition for the late turn-execution and reporting body. Keep the
// historical aliases live through part_016 exactly as the translation-unit root
// did before this structural extraction.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition root: https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp

#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "../part_014c.inc"
#undef ability_available_for_pokemon

// part_014c has already declared the historical scenario_by_label() and leaves
// only its final return/brace for the first two lines of part_015. The Garbodor
// wrapper was composed at namespace scope through part_forretress_ex_combo.inc.
// Route reporting, self-test lookups, and the production CLI through the appended
// rows while leaving the historical public registry intact for focused tests:
// https://api.pokemontcg.io/v2/cards/xy9-57
// https://api.pokemontcg.io/v2/cards/swsh7-142
// https://github.com/FlareZ123/pokemon-sims/issues/2808
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for the final implementation stage in part_016.inc.

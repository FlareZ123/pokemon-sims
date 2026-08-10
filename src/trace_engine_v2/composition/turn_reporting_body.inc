#pragma once

// Ordered composition for the late turn-execution and reporting body. Keep the
// historical aliases live through part_016 exactly as the translation-unit root
// did before this structural extraction.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition root: https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp

#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "../part_014c.inc"
#include "../part_015.inc"

// play_field_blower and run_turn intentionally remain aliased for the final
// implementation stage in part_016.inc.

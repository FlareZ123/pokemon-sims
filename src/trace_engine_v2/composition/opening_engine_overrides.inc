#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "../part_begin_turn_override.inc"
#include "../part_tapu_copy_availability_override.inc"
#include "opening_bench_aliases.inc"

// choose_discard intentionally remains exported through part_007.inc and
// part_008a.inc; src/regidrago_sim.cpp releases it at that legacy boundary.

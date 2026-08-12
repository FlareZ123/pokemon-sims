#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// K0/K1 deck visibility and deck-knowledge policy live in the canonical core unit:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_knowledge.inc
#include "../core/deck_knowledge.inc"
#undef begin_turn
// Turn lifecycle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/turn_lifecycle.inc
#include "../core/turn_lifecycle.inc"
// Tapu connector policy now owns its adjacent historical aliases as one core unit.
#include "../core/tapu_connector_policy.inc"
#include "../core/payload_hand_policy.inc"

// Opening Bench, discard, and Tate aliases are centralized behind one composition
// boundary while preserving their historical textual include order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_turn_composition.inc
#include "opening_legacy_turn_composition.inc"

// Search-item, Steven/Blender, Tapu/FSS, and legacy Supporter/VSTAR wrappers are
// centralized behind the adjacent connector boundary. That unit preserves the
// historical macro handoffs, including play_ultra_ball and the late use_fss aliases.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_connector_composition.inc
#include "opening_connector_composition.inc"

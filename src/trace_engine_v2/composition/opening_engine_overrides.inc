#pragma once

// Ordered composition for the early Engine override layer. This file deliberately
// preserves macro state across the split so later legacy parts see the same aliases
// they saw when these directives lived in src/regidrago_sim.cpp.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "../part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "../part_begin_turn_override.inc"
#include "../core/tapu_connector_policy.inc"
#include "../core/payload_hand_policy.inc"
#include "../core/tapu_legacy_aliases.inc"

// Keep the coupled part_006/007/008a alias lifetime in one named boundary before
// the later override stages take ownership.
// Composition source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_body.inc
#include "opening_legacy_body.inc"

// The tail wrapper preserves the historical Tate/discard -> Steven/Blender ->
// Supporter -> VSTAR order and the aliases that intentionally survive to part_014a.
// Composition source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_override_tail.inc
#include "opening_override_tail.inc"

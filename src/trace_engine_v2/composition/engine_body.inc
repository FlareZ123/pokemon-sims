#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// This wrapper centralizes the textual include order without changing macro
// lifetime or member boundaries. Several legacy parts deliberately open a member
// that the next stage closes, so preserve this sequence exactly.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core engine stages: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_engine_body.inc
// Opening through VSTAR: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search overrides: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late turn/reporting stages: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/turn_reporting_body.inc

// Core Engine state, turn, trace body, and Garbotoxin-aware Ability wrapper.
// begin_turn intentionally remains aliased until opening_engine_overrides.inc
// composes part_empty_deck_unseen_override.inc and releases the alias.
#include "core_engine_body.inc"
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#include "post_014a_overrides.inc"

// Preserve the late alias lifetime and translation-unit closure in one named stage.
#include "turn_reporting_body.inc"

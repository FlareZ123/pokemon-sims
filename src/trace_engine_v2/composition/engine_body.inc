#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// This wrapper centralizes the textual include order without changing macro
// lifetime or member boundaries. Several legacy parts deliberately open a member
// that the next stage closes, so preserve this sequence exactly.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core engine: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_engine_body.inc
// Opening through VSTAR: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search overrides: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late turn execution: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
// Reporting body: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

#include "core_engine_body.inc"
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#include "post_014a_overrides.inc"

// Preserve the historical aliases through the final implementation stage. These
// two legacy parts form the final contiguous Engine member body before part_016.
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "../part_014c.inc"
#include "../part_015.inc"

// play_field_blower and run_turn intentionally remain aliased for part_016.inc.
#include "../part_016.inc"

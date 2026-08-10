#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// This wrapper centralizes the textual include order without changing macro
// lifetime or member boundaries. Several legacy parts deliberately open a member
// that the next stage closes, so preserve this sequence exactly.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core engine stages: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_000.inc
// Opening through VSTAR: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search overrides: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late turn stages: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

// Core Engine state, turn, and trace body. This keeps the legacy macro lifetime
// consumed by the opening override layer while removing one include-only wrapper.
#include "../part_000.inc"
#include "../part_001.inc"
#include "../part_002.inc"

#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"

// part_003.inc opens begin_turn(), part_004.inc completes it and later opens
// state_line(), and part_005.inc closes state_line() before the first override
// member is introduced:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L151-L154
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L1-L22
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L218
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc#L1-L5
#include "../part_004.inc"
#include "../part_005.inc"

#undef might_be_unseen
// begin_turn intentionally remains aliased until opening_engine_overrides.inc
// composes part_empty_deck_unseen_override.inc and releases the alias.
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#include "post_014a_overrides.inc"

// Late turn-execution and reporting stages. Keep these aliases live through
// part_016 exactly as the former wrapper did.
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "../part_014c.inc"
#include "../part_015.inc"

// play_field_blower and run_turn intentionally remain aliased for part_016.inc.
#include "../part_016.inc"
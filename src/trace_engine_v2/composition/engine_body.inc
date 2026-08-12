#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core and late-reporting stages live in named composition units so the legacy
// forwarding wrappers are unnecessary. Preserve this textual include sequence and
// the macro lifetimes because several legacy parts deliberately cross boundaries.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Opening core composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_core.inc
// Banked-Tapu/search closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_override.inc
// Post-search lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "core_state.inc"
#include "opening_core.inc"

// begin_turn intentionally remains aliased until opening_engine_overrides.inc,
// composed by banked_tapu_override.inc, releases the alias.
#include "banked_tapu_override.inc"
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

#pragma once

// Single ordered composition entry point for the simulator Engine implementation.
// Keep the canonical stage order here so the root translation unit has one stable
// implementation entry point without an additional include-only pipeline wrapper.
// Preserve this textual sequence because legacy aliases deliberately cross
// composition boundaries before their owning wrappers release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Opening core: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_core.inc
// Banked-Tapu/search closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_override.inc
// Post-search lock removal: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "core_state.inc"
#include "opening_core.inc"

// begin_turn intentionally remains aliased until opening_engine_overrides.inc,
// composed by banked_tapu_override.inc, releases the alias.
#include "banked_tapu_override.inc"
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

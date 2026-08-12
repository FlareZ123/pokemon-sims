#pragma once

// Ordered continuation after engine_body.inc composes core_state.inc and
// opening_core.inc. Keep this sequence intact while the wrapper is consolidated.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Banked-Tapu/search closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_override.inc
// Post-search lock removal: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

// begin_turn intentionally remains aliased until opening_engine_overrides.inc,
// composed by banked_tapu_override.inc, releases the alias.
#include "banked_tapu_override.inc"
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"
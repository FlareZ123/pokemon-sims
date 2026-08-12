#pragma once

// Ordered opening and late Engine composition pipeline. These stages intentionally
// stay in this textual order because legacy aliases cross composition boundaries
// before their owning wrappers release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Opening core: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_core.inc
// Banked-Tapu/search closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_override.inc
// Post-search lock removal: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "opening_core.inc"

// begin_turn intentionally remains aliased until opening_engine_overrides.inc,
// composed by banked_tapu_override.inc, releases the alias.
#include "banked_tapu_override.inc"
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

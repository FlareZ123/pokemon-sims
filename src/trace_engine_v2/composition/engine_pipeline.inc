#pragma once

// Ordered late continuation after engine_body.inc composes the base, opening,
// and banked-Tapu stages. Keep this pair intact while the wrapper is consolidated.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Post-search lock removal: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"
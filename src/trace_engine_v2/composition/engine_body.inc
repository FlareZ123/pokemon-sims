#pragma once

// Ordered composition spine for the simulator Engine implementation. Each unit owns
// one contiguous textual-include lifecycle so macro boundaries stay local while the
// historical member order remains unchanged.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Early core and legacy members: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/early_engine.inc
// Opening/search banked-Tapu bridge: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_bridge.inc
// Post-search lock removal: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "early_engine.inc"
#include "banked_tapu_bridge.inc"
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

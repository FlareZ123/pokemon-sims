#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core and late-reporting stages live in named composition units so the legacy
// forwarding wrappers are unnecessary. Preserve this textual include sequence and
// the macro lifetimes because several legacy parts deliberately cross boundaries.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Opening, search, Steven/Blender, Supporter, and VSTAR composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "core_state.inc"
#include "early_engine.inc"
#include "banked_tapu_bridge.inc"
#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

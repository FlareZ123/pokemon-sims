#pragma once

// Single ordered composition entry point for the simulator Engine implementation.
// Own the base state/opening stages here while the remaining legacy continuation
// stays temporarily grouped in engine_pipeline.inc during wrapper consolidation.
// Preserve this textual include order because legacy aliases deliberately cross
// composition boundaries before their owning wrappers release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Opening core: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_core.inc
// Remaining continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_pipeline.inc

#include "core_state.inc"
#include "opening_core.inc"
#include "engine_pipeline.inc"
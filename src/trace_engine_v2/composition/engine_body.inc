#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core/opening stages remain here while the late Engine chain lives in one named
// pipeline. Preserve textual include order and macro lifetimes across the boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Opening core: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_core.inc
// Late Engine pipeline: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_pipeline.inc

#include "core_state.inc"
#include "opening_core.inc"
#include "engine_pipeline.inc"

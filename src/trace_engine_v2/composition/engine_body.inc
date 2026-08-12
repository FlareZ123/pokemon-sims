#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core state remains here while the opening and late Engine stages live in one
// named pipeline. Preserve textual include order and macro lifetimes throughout.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Engine pipeline: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_pipeline.inc

#include "core_state.inc"
#include "engine_pipeline.inc"

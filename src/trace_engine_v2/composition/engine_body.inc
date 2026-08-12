#pragma once

// Single ordered composition entry point for the simulator Engine implementation.
// The complete textual include chain lives in engine_pipeline.inc so macro lifetimes
// remain centralized and the root body cannot drift from the canonical order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Engine pipeline: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_pipeline.inc

#include "engine_pipeline.inc"

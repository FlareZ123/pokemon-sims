#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Named composition stages own the legacy textual include ordering and macro
// lifetimes so this file stays at pipeline level.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core bootstrap: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_bootstrap.inc
// Opening/search pipeline: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_pipeline.inc
// Post-search execution pipeline: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_pipeline.inc

#include "core_bootstrap.inc"
#include "opening_pipeline.inc"
#include "post_search_pipeline.inc"

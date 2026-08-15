#pragma once

// Canonical state and card bootstrap for the Engine composition entry point.
// Keep the include guards here so tests that reuse the translation unit preserve
// the historical one-definition boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

#ifndef SIM_TRACE_SIMULATOR_STATE_INCLUDED
#define SIM_TRACE_SIMULATOR_STATE_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "../core/card_catalog.inc" // Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
#endif

#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "../core/card_classification.inc" // Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
#endif

#include "../core/simulator_state.inc" // Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc

#endif  // SIM_TRACE_SIMULATOR_STATE_INCLUDED

#pragma once

// Centralized card-model composition for simulator state.
// Keep the historical include guards in one place so state composition does not
// duplicate catalog/classification ownership details.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "../core/card_catalog.inc"
#endif

#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "../core/card_classification.inc"
#endif
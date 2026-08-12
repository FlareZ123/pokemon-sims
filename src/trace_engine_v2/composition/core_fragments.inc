#pragma once

// Core fragments are textually composed inside Engine. Keep the historical include
// guards because legacy focused tests may still include these fragments directly.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#ifndef SIM_TRACE_SIMULATOR_STATE_INCLUDED
#define SIM_TRACE_SIMULATOR_STATE_INCLUDED

// Card metadata precedes the state model because simulator_state.inc consumes both
// the catalog and the shared classification predicates.
#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "../core/card_catalog.inc"
#endif

#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "../core/card_classification.inc"
#endif

// Compose the state definition last, at the same Engine member boundary previously
// occupied by the three inline includes in engine_body.inc.
#include "../core/simulator_state.inc"

#endif  // SIM_TRACE_SIMULATOR_STATE_INCLUDED

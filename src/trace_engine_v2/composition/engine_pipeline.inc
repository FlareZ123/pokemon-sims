#pragma once

// Canonical ordered composition pipeline for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// composition boundaries before their owning wrappers release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Opening composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_composition.inc
// Lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/lock_removal_composition.inc
// Late execution/reporting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#ifndef SIM_TRACE_SIMULATOR_STATE_INCLUDED
#define SIM_TRACE_SIMULATOR_STATE_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "../core/card_catalog.inc"
#endif

#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "../core/card_classification.inc"
#endif

#include "../core/simulator_state.inc"

#endif  // SIM_TRACE_SIMULATOR_STATE_INCLUDED

#include "opening_composition.inc"
#include "lock_removal_composition.inc"
#include "late_execution_reporting.inc"

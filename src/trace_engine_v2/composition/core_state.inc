#pragma once

// Centralized Engine state composition. Keep the historical guards and include order
// together so the root composition spine does not duplicate core ownership details.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card model composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/card_model.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
#ifndef SIM_TRACE_SIMULATOR_STATE_INCLUDED
#define SIM_TRACE_SIMULATOR_STATE_INCLUDED

#include "card_model.inc"
#include "../core/simulator_state.inc"

#endif  // SIM_TRACE_SIMULATOR_STATE_INCLUDED
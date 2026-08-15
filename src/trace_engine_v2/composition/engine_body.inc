#pragma once

// Canonical ordered composition entry point for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// stage boundaries before their owning sections release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

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

// Opening stage. The named composition unit owns the historical member fragments
// and their temporary aliases while preserving the exact textual include order.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_stage.inc
#include "opening_legacy_stage.inc"

// Post-search stage. The named composition unit owns the banked-Tapu replacement
// and the temporary alias that spans the historical search-item continuation.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_stage.inc
#include "banked_tapu_stage.inc"

// Lock-removal stage. The named composition unit owns shared Forest/Field Blower
// behavior and the temporary aliases required by the post-search continuation.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/lock_removal_stage.inc
#include "lock_removal_stage.inc"

// Late stage. The named composition unit owns the turn-execution, scenario-registry,
// reporting fragments, and aliases that intentionally span their historical files.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_legacy_stage.inc
#include "late_legacy_stage.inc"

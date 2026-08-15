#pragma once

// Canonical ordered composition entry point for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// stage boundaries before their owning sections release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Banked-Tapu policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_policy_stage.inc
// Lock-removal policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/lock_removal_policy_stage.inc
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

// Middle stage. Compose the two macro-owning policy substages directly at the
// canonical Engine boundary so the forwarding-only middle_policy_stage.inc layer
// cannot drift from the actual banked-Tapu -> lock-removal sequence.
// Banked-Tapu owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_policy_stage.inc
#include "banked_tapu_policy_stage.inc"
// Lock-removal owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/lock_removal_policy_stage.inc
#include "lock_removal_policy_stage.inc"

// Late stage. Compose the two macro-owning substages directly at the canonical
// Engine boundary so the forwarding-only late_legacy_stage.inc layer is unnecessary.
// Execution owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_stage.inc
#include "late_execution_stage.inc"
// Registry/reporting owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc
#include "late_registry_reporting_stage.inc"

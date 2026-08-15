#pragma once

// Canonical ordered composition entry point for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// stage boundaries before their owning sections release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Lock-removal policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/lock_removal_policy_stage.inc
// Late composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_engine_stage.inc
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

// Middle stage: Banked-Tapu policy at the historical post-search member boundary.
// Keep macro setup/include/teardown order and the local entry/exit assertions here,
// beside the canonical composition boundary that owns the sequence.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// DDE-aware Apex payment / confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/3089
// Search-item runtime owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/turn_action_policy_runtime.inc
// Replacement policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/banked_tapu_retreat_policy.inc
#if defined(retreat_banked_tapu_to_regidrago)
#error "Banked-Tapu composition requires the retreat alias to be undefined on entry"
#endif
#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../turn_action_policy_runtime.inc"
#undef retreat_banked_tapu_to_regidrago
#if defined(retreat_banked_tapu_to_regidrago)
#error "Banked-Tapu composition leaked the temporary retreat alias"
#endif
#if defined(begin_turn)
#error "Banked-Tapu composition expected opening_engine_overrides.inc to release begin_turn"
#endif
#include "../core/routes/banked_tapu_retreat_policy.inc"

// Lock-removal owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/lock_removal_policy_stage.inc
#include "lock_removal_policy_stage.inc"

// Late stage. The execution aliases deliberately span part_014c through part_016,
// so one composition owner now holds their complete textual lifetime.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_engine_stage.inc
#include "late_engine_stage.inc"

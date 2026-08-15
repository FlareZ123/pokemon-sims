#pragma once

// Own the ordered late turn-execution, scenario-registry, and reporting fragments
// as one textual composition stage. Named substages keep each alias-lifetime seam
// explicit while preserving the historical part_014c -> part_015 -> part_016 order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Execution owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_stage.inc
// Registry/reporting owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc

#include "late_execution_stage.inc"

// The execution substage intentionally leaves the Field Blower alias live for the
// registry/reporting continuation. part_014c itself releases the run_turn alias
// before composing the replacement Latias-aware run_turn implementation, while the
// execution substage releases its local item-loop and Garbodor Ability aliases.
// Handoff sources: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc
#if !defined(play_field_blower)
#error "Late execution must export play_field_blower"
#endif
#if defined(run_turn) || defined(hand_count) || defined(play_items_until_stable) || \
    defined(ability_available_for_pokemon)
#error "Late execution leaked a released alias into registry/reporting composition"
#endif

#include "late_registry_reporting_stage.inc"

#pragma once

// Own the ordered late turn-execution, scenario-registry, and reporting fragments
// as one textual composition stage. Named substages keep each alias-lifetime seam
// explicit while preserving the historical part_014c -> part_015 -> part_016 order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Execution owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_stage.inc
// Registry/reporting owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc

#include "late_execution_stage.inc"

// The execution substage intentionally exports only the aliases consumed by the
// registry/reporting continuation. Its local item-loop aliases and the long-lived
// Garbodor Ability wrapper must already be released at this boundary.
// Handoff sources: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc
#if !defined(play_field_blower) || !defined(run_turn)
#error "Late execution must export play_field_blower and run_turn"
#endif
#if defined(hand_count) || defined(play_items_until_stable) || \
    defined(ability_available_for_pokemon)
#error "Late execution leaked a local alias into registry/reporting composition"
#endif

#include "late_registry_reporting_stage.inc"

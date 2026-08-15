#pragma once

// Own the ordered late turn-execution, scenario-registry, and reporting fragments
// as one textual composition stage. Named substages keep each alias-lifetime seam
// explicit while preserving the historical part_014c -> part_015 -> part_016 order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Execution owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_stage.inc
// Registry/reporting owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc

#include "late_execution_stage.inc"

// The execution substage consumes and releases its temporary wrapper aliases before
// the namespace-scope registry/reporting continuation begins. Keep this boundary
// explicit so a future extraction cannot accidentally leak a class-body alias.
// Historical execution source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable) || defined(ability_available_for_pokemon)
#error "Late execution leaked a temporary alias into registry/reporting composition"
#endif

#include "late_registry_reporting_stage.inc"

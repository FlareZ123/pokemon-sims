#pragma once

// Own the ordered late turn-execution, scenario-registry, and reporting fragments
// as one textual composition stage. Named substages keep each alias-lifetime seam
// explicit while preserving the historical part_014c -> part_015 -> part_016 order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Execution owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_stage.inc
// Registry/reporting owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_registry_reporting_stage.inc

#include "late_execution_stage.inc"
#include "late_registry_reporting_stage.inc"

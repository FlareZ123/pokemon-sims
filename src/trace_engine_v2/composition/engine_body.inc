#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Named composition stages own the legacy textual include ordering and macro
// lifetimes so this file stays at pipeline level.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core bootstrap: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_bootstrap.inc
// Opening, search, Steven/Blender, Supporter, and VSTAR composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search lock-removal composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_search_lock_removal.inc
// Late execution/reporting composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "core_bootstrap.inc"

// Replace only the historical banked-Tapu implementation while preserving the
// rest of the opening override ordering. The replacement uses the shared DDE-aware
// Apex payment predicate: https://github.com/FlareZ123/pokemon-sims/issues/3089
#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#undef retreat_banked_tapu_to_regidrago
// part_014a closes run_search_items_one_step(), so the completed replacement member
// is composed directly from core at the same safe class-member boundary:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/banked_tapu_retreat_policy.inc
// https://github.com/FlareZ123/pokemon-sims/issues/3089
#include "../core/banked_tapu_retreat_policy.inc"

#include "post_search_lock_removal.inc"
#include "late_execution_reporting.inc"

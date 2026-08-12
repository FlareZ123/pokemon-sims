#pragma once

// Centralize the banked-Tapu override's macro lifetime and replacement member.
// Preserve this order because part_014a closes run_search_items_one_step() before
// the completed replacement policy is composed at the safe class-member boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// DDE-aware Apex payment / confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/3089
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Replacement policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/banked_tapu_retreat_policy.inc
#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#undef retreat_banked_tapu_to_regidrago
#include "../core/banked_tapu_retreat_policy.inc"
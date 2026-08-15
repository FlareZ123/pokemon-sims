#pragma once

// Own the banked-Tapu replacement and its temporary retreat alias at the
// post-search member boundary. Preserve this include order because
// turn_action_policy_runtime.inc closes run_search_items_one_step() before the
// completed replacement policy is composed at the safe class-member boundary.
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

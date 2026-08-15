// Middle composition stage: banked-Tapu replacement followed by lock-removal wiring.
// This file is textually included from engine_body.inc at the historical boundary.
// Preserve macro setup/include/teardown order; the entry and exit assertions below
// make the temporary-alias contract local to this composition owner.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

// Phase 3: compose the banked-Tapu replacement at the post-search member boundary.
// Preserve this order because turn_action_policy_runtime.inc closes
// run_search_items_one_step() before the completed replacement policy is composed
// at the safe class-member boundary.
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

// Load the shared forest / Field Blower policy before exporting the two aliases
// required by post-search composition.
// Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
// Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
// Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
#include "../core/forest_field_blower_policy.inc"

#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition requires lock-removal aliases to be undefined"
#endif
#define play_chaotic_swell play_chaotic_swell_issue_3057
#define play_field_blower play_field_blower_issue_3057
#include "post_014a_overrides.inc"
#undef play_field_blower
#undef play_chaotic_swell

#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition leaked a temporary lock-removal alias"
#endif

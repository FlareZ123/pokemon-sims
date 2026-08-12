#pragma once

// Ordered opening/search composition boundary. The core bootstrap intentionally
// exports begin_turn and the Garbotoxin-aware Ability alias into this stage.
// The banked-Tapu compatibility alias is local to this unit and is released
// immediately after part_014a closes the legacy search-item member.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// DDE-aware banked-Tapu policy: https://github.com/FlareZ123/pokemon-sims/issues/3089

#if defined(retreat_banked_tapu_to_regidrago)
#error "Opening pipeline requires retreat_banked_tapu_to_regidrago to be undefined on entry"
#endif

#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#undef retreat_banked_tapu_to_regidrago

// part_014a closes run_search_items_one_step(), so the completed replacement member
// is composed directly from core at the same safe class-member boundary:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/banked_tapu_retreat_policy.inc
#include "../core/banked_tapu_retreat_policy.inc"

#if defined(retreat_banked_tapu_to_regidrago)
#error "Opening pipeline leaked retreat_banked_tapu_to_regidrago"
#endif

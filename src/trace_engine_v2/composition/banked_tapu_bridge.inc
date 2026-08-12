#pragma once

// Compose the opening overrides through the post-search banked-Tapu replacement.
// The macro preserves the historical member while the DDE-aware replacement is
// emitted at the same stable class-member boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// DDE-aware banked-Tapu policy: https://github.com/FlareZ123/pokemon-sims/issues/3089
#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#undef retreat_banked_tapu_to_regidrago

// part_014a closes run_search_items_one_step(), so the replacement member is
// composed directly from core after the original alias is released.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/banked_tapu_retreat_policy.inc
// https://github.com/FlareZ123/pokemon-sims/issues/3089
#include "../core/banked_tapu_retreat_policy.inc"

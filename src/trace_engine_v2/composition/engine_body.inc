#pragma once

// Canonical ordered composition entry point for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// stage boundaries before their owning sections release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Lock-removal policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forest_field_blower_policy.inc
// Lock-removal specification: https://github.com/FlareZ123/pokemon-sims/issues/3057
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

#ifndef SIM_TRACE_SIMULATOR_STATE_INCLUDED
#define SIM_TRACE_SIMULATOR_STATE_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "../core/card_catalog.inc"
#endif

#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "../core/card_classification.inc"
#endif

#include "../core/simulator_state.inc"

#endif  // SIM_TRACE_SIMULATOR_STATE_INCLUDED

// Opening stage. The named composition unit owns the historical member fragments
// and their temporary aliases while preserving the exact textual include order.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_stage.inc
#include "opening_legacy_stage.inc"

// Banked-Tapu stage. The named composition unit owns the post-search member
// boundary and its temporary retreat alias.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/banked_tapu_stage.inc
#include "banked_tapu_stage.inc"

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

// Late stage. The named composition unit owns the turn-execution, scenario-registry,
// reporting fragments, and aliases that intentionally span their historical files.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_legacy_stage.inc
#include "late_legacy_stage.inc"

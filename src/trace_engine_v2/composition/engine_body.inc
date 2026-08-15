#pragma once

// Canonical ordered composition entry point for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// stage boundaries before their owning sections release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Banked-Tapu replacement policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/banked_tapu_retreat_policy.inc
// Lock-removal policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forest_field_blower_policy.inc
// Battle Compressor / VS Seeker policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/battle_compressor_vs_seeker_policy.inc
// Late composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_engine_stage.inc
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

// Opening stage: own the historical member fragments and temporary aliases at the
// canonical Engine composition boundary. Keeping the setup, continuations, and
// teardown here removes a forwarding-only .inc layer while preserving include order.
// Own the historical opening fragments and their temporary aliases as one textual
// composition stage. This preserves the legacy member-continuation boundary while
// keeping macro lifetime out of the canonical engine-body overview.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical opening fragment: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
// Historical continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc
// State-line closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc
// Garbotoxin policy owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/garbodor_lock_policy.inc

#if defined(begin_turn) || defined(might_be_unseen)
#error "Opening composition requires begin_turn and might_be_unseen aliases to be undefined on entry"
#endif
#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"

#if defined(ability_available_for_pokemon)
#error "Garbodor composition requires ability_available_for_pokemon to be undefined on entry"
#endif
#define ability_available_for_pokemon ability_available_for_pokemon_garbodor

// part_003.inc opens begin_turn(); part_004.inc completes it and later opens
// state_line(); part_005.inc closes state_line() before the first override member.
// Keep these continuations adjacent so the real textual boundary and alias lifetime
// are visible in one composition owner rather than hidden behind a forwarding stage.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L151-L154
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L1-L22
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L218
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc#L1-L5
#include "../part_004.inc"
#include "../part_005.inc"
#include "../core/garbodor_lock_policy.inc"

#undef might_be_unseen
#if defined(might_be_unseen)
#error "Opening composition leaked the temporary might_be_unseen alias"
#endif

// Middle stage: own the banked-Tapu alias lifetime directly at the canonical
// Engine boundary. This removes a forwarding-only .inc layer while preserving the
// exact macro setup/include/teardown sequence.
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

// Own lock-removal aliases at this same canonical post-search boundary instead of
// forwarding through another composition file.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
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

// Battle Compressor and VS Seeker must be composed after the current search/DCI
// policy stack but before part_014c binds the final Item fixed point.
// Exact Items: https://api.pokemontcg.io/v2/cards/xy4-92 https://api.pokemontcg.io/v2/cards/xy4-109
// Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
#include "../core/battle_compressor_vs_seeker_policy.inc"

// Late stage. The execution aliases deliberately span part_014c through part_016,
// so one composition owner now holds their complete textual lifetime.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_engine_stage.inc
#include "late_engine_stage.inc"
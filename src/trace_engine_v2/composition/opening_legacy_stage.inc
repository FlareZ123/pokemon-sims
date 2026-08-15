#pragma once

// Own the historical opening fragments and their temporary aliases as one textual
// composition stage. This preserves the legacy member-continuation boundary while
// keeping macro lifetime out of the canonical engine-body overview.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical opening fragment: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
// Opening completion owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_state_completion_stage.inc

#if defined(begin_turn) || defined(might_be_unseen)
#error "Opening composition requires begin_turn and might_be_unseen aliases to be undefined on entry"
#endif
#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"

// part_003.inc opens begin_turn(); the named completion substage preserves the
// historical continuation and macro lifetime at this exact textual boundary.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L151-L154
#include "opening_state_completion_stage.inc"

// These wrappers intentionally continue into later canonical stages. Assert that
// the opening owner exported both aliases so future fragment cleanup cannot silently
// shorten either historical lifetime.
// begin_turn continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Ability continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_engine_stage.inc
#if !defined(begin_turn) || !defined(ability_available_for_pokemon)
#error "Opening composition must export begin_turn and ability_available_for_pokemon"
#endif

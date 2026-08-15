// Complete the historical opening chain after part_003.inc opens begin_turn().
// This substage owns the Garbodor ability alias, part_004/part_005 continuation,
// and release of the temporary opening-deck visibility alias.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc
// State-line closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc
// Garbotoxin policy owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/garbodor_lock_policy.inc

#if defined(ability_available_for_pokemon)
#error "Garbodor composition requires ability_available_for_pokemon to be undefined on entry"
#endif
#define ability_available_for_pokemon ability_available_for_pokemon_garbodor

// part_004.inc completes begin_turn() and later opens state_line(); part_005.inc
// closes state_line() before the first override member is introduced.
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

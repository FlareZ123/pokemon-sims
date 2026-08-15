// Compose the historical late turn-execution fragment and keep its temporary alias
// lifetime contained before the namespace-scope registry/reporting substage begins.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical turn/scenario fragment: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808

#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable)
#error "Late-turn composition requires execution aliases to be undefined on entry"
#endif
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#define hand_count hand_count_issue_3057
#define play_items_until_stable play_items_until_stable_issue_3057
#include "../part_014c.inc"
#undef play_items_until_stable
#undef hand_count
#undef ability_available_for_pokemon

// part_014c.inc consumes and releases play_field_blower/run_turn internally; the
// remaining wrappers are released above. Since engine_body.inc now composes the
// registry/reporting substage directly next, reject any accidental alias leakage.
// Direct composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable) || defined(ability_available_for_pokemon)
#error "Late execution leaked a temporary alias into registry/reporting composition"
#endif

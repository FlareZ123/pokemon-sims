#pragma once

// Own the complete historical late composition boundary in one textual stage.
// The aliases created for part_014c intentionally stay live through part_015 and
// part_016, so keeping their setup and teardown together makes that lifetime explicit.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Historical execution fragment: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
// Historical registry continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
// Battle Compressor / VS Seeker policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/battle_compressor_vs_seeker_policy.inc
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
#define play_items_until_stable play_items_until_stable_issue3545
#include "../part_014c.inc"
#undef play_items_until_stable
#undef hand_count
#undef ability_available_for_pokemon

// part_014c defines the historical scenario registry and leaves only the final
// return/brace of scenario_by_label() for the first two lines of part_015. Keep
// play_field_blower and run_turn live until part_016 closes the legacy chain.
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition requires aliases to be undefined on entry"
#endif
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"
#include "../part_016.inc"

#undef scenario_by_label
#undef all_scenarios
#undef run_turn
#undef play_field_blower

#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable) || defined(all_scenarios) || \
    defined(scenario_by_label)
#error "Canonical engine composition leaked a late alias"
#endif

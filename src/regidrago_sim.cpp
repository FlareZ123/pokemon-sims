// Split only for reviewable GitHub contents-API commits. The compiler receives one translation unit.
//
// Direct rules/card-text registry. Method-level mappings and validation notes:
// docs/RULE_SOURCES.md and docs/RULES_TRACEABILITY.md.
//
// Composition map for this translation-unit root:
// 1. part_000..005 establish core state, turn, and trace methods.
// 2. opening_engine_overrides composes the early-turn wrappers.
// 3. tate_legacy_body keeps the historical Tate & Liza body isolated.
// 4. tate_discard_overrides owns the current Tate discard policy chain.
// 5. steven_blender_overrides composes Steven and Brilliant Blender policy.
// 6. legacy_supporter_body owns the part_011 legacy supporter aliases.
// 7. supporter_selector_body owns the part_012 supporter-selector aliases.
// 8. vstar_power_body owns the part_013 VSTAR-power aliases.
// 9. part_014a closes the search-item member opened by part_013.
// 10. post_014a_overrides composes Burnet, FSS, Vessel, and later wrappers.
// 11. part_014c and part_015 complete turn execution and reporting.
// 12. part_016 closes the translation-unit implementation.
// Composition wrappers intentionally preserve macro lifetime where a later stage
// consumes an alias; see each composition file for the matching continuation URL.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core procedure rules: https://www.pokemon.com/us/pokemon-tcg/rules
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
// Dawn: https://api.pokemontcg.io/v2/cards/me2-87
// Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
// Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
// Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
// Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
// Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
// Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
// Confirmed source-registry fix: https://github.com/FlareZ123/pokemon-sims/issues/856
// Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123 ; paper Expanded enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2509
// Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2238
// Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
// Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
// Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
// Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Serena draw mode requires discarding at least 1 card before drawing to 5: https://api.pokemontcg.io/v2/cards/swsh12-164
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
// Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
// Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
// Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
// Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
// Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
// Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
// Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
#include "trace_engine_v2/part_000.inc"
#include "trace_engine_v2/part_001.inc"
#include "trace_engine_v2/part_002.inc"
#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "trace_engine_v2/part_003.inc"
// part_003.inc opens begin_turn(), and part_004.inc completes it. part_004.inc
// later opens state_line(), which part_005.inc completes before a new Engine
// member may be defined:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L151-L154
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L1-L22
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L218
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc#L1-L5
#include "trace_engine_v2/part_004.inc"
#include "trace_engine_v2/part_005.inc"
#undef might_be_unseen
#include "trace_engine_v2/composition/opening_engine_overrides.inc"
#include "trace_engine_v2/composition/tate_legacy_body.inc"
#include "trace_engine_v2/composition/tate_discard_overrides.inc"
// base_search_overrides.inc is composed by tate_discard_overrides.inc.
#include "trace_engine_v2/composition/steven_blender_overrides.inc"
// tapu_search_overrides.inc is composed by steven_blender_overrides.inc.
#include "trace_engine_v2/composition/legacy_supporter_body.inc"
#include "trace_engine_v2/composition/supporter_selector_body.inc"
#include "trace_engine_v2/composition/vstar_power_body.inc"
#include "trace_engine_v2/part_014a.inc"
// part_012.inc opens Serena's draw-mode body, part_013.inc closes it and later
// opens run_search_items_one_step(), and part_014a.inc completes that method.
// Define the active Burnet policy only after this first complete member boundary:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_012.inc#L212-L228
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013.inc#L1-L20
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013.inc#L205-L224
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc#L1-L20
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
// The active thinning policy is the implementation wrapped by the later empty-deck
// guard, while the legacy part_011 implementation stays dormant:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011_burnet_thinning_override.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_empty_deck_search_override.inc#L102-L107
#include "trace_engine_v2/composition/post_014a_overrides.inc"
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "trace_engine_v2/part_014c.inc"
#include "trace_engine_v2/part_015.inc"
// C++ preprocessing include grammar: https://eel.is/c++draft/cpp.include
// Confirmed portability bug: https://github.com/FlareZ123/pokemon-sims/issues/1482
#include "trace_engine_v2/part_016.inc"
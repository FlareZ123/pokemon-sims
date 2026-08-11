// Split only for reviewable GitHub contents-API commits. The compiler receives one translation unit.
//
// Direct rules/card-text registry. Method-level mappings and validation notes:
// docs/RULE_SOURCES.md and docs/RULES_TRACEABILITY.md.
//
// The named engine_body wrapper owns the ordered implementation composition and
// preserves the legacy macro/member continuation boundaries in one place.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition spine: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
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
//
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Burnet/search continuation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late alias handoff: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
//
// Source-anchor compatibility block.
// Several generated documentation checks resolve same-repository line anchors.
// Keep the composition include at line 104 unless those checked anchors are migrated.
// Contract test: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/update_setup_docs_tests.py
// Composition wrapper: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
//
// Engine body stage registry:
// Core engine body: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
// Opening overrides: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search overrides: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Turn/reporting body: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
//
// Opening continuation registry:
// Opening legacy body: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Opening override tail: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
//
// Rules traceability:
// Rules registry: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/RULE_SOURCES.md
// Rules traceability: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/RULES_TRACEABILITY.md
// Policy decisions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
// Model assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
//
// Stable direct card-data sources used by composed stages:
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
//
// Procedure sources:
// Official Pokémon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
//
// The following include is intentionally the single implementation entry point.
// It preserves the ordered textual continuation boundaries documented above.
// Same-repository anchor consumers currently target this footer.
// Keep these three footer lines together.
// Engine composition entry point follows.
// Composition wrapper source is linked above.
// The include itself remains behaviorally identical to the former root sequence.
//
// Engine body include:
#include "trace_engine_v2/composition/engine_body.inc"
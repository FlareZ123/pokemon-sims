// Split only for reviewable GitHub contents-API commits. The compiler receives one translation unit.
//
// Direct rules/card-text registry. Method-level mappings and validation notes:
// docs/RULE_SOURCES.md and docs/RULES_TRACEABILITY.md.
// Core procedure rules: https://www.pokemon.com/us/pokemon-tcg/rules
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
// Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
// Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
// Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Serena draw mode requires discarding at least 1 card before drawing to 5: https://api.pokemontcg.io/v2/cards/swsh12-164
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
// Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
#include "trace_engine_v2/part_000.inc"
#include "trace_engine_v2/part_001.inc"
#include "trace_engine_v2/part_002.inc"
#include "trace_engine_v2/part_003.inc"
#include "trace_engine_v2/part_004.inc"
#include "trace_engine_v2/part_005.inc"
#include "trace_engine_v2/part_006.inc"
#include "trace_engine_v2/part_007.inc"
#include "trace_engine_v2/part_008a.inc"
#include "trace_engine_v2/part_008b.inc"
#include "trace_engine_v2/part_009a.inc"
#include "trace_engine_v2/part_009b1.inc"
#include "trace_engine_v2/part_009b2.inc"
#include "trace_engine_v2/part_010.inc"
#include "trace_engine_v2/part_011.inc"
#define play_arven play_arven_original
#include "trace_engine_v2/part_012.inc"
#undef play_arven
#include "trace_engine_v2/part_013.inc"
#include "trace_engine_v2/part_014a.inc"
#include "trace_engine_v2/part_012_override.inc"
#include "trace_engine_v2/part_014b.inc"
#include "trace_engine_v2/part_014c.inc"
#include "trace_engine_v2/part_015.inc"
#include "trace_engine_v2/part_016.inc"

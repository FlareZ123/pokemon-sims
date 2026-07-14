// Split only for reviewable GitHub contents-API commits. The compiler receives one translation unit.
//
// Direct rules/card-text registry. Method-level mappings and validation notes:
// docs/RULE_SOURCES.md and docs/RULES_TRACEABILITY.md.
// Core procedure rules: https://www.pokemon.com/us/pokemon-tcg/rules
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
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
// Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
// Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
// Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
// Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
// Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
#include "trace_engine_v2/part_000.inc"
#include "trace_engine_v2/part_001.inc"
#include "trace_engine_v2/part_002.inc"
#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "trace_engine_v2/part_003.inc"
#undef might_be_unseen
#include "trace_engine_v2/part_empty_deck_unseen_override.inc"
#include "trace_engine_v2/part_004.inc"
#include "trace_engine_v2/part_005.inc"
#undef begin_turn
#include "trace_engine_v2/part_begin_turn_override.inc"
#define bench_from_hand bench_from_hand_empty_deck_original
#define bench_oricorio_if_useful bench_oricorio_if_useful_empty_deck_original
#define needs_tapu_connector needs_tapu_connector_original
#define bench_tapu_if_useful bench_tapu_if_useful_original
#include "trace_engine_v2/part_006.inc"
#undef bench_tapu_if_useful
#undef needs_tapu_connector
#undef bench_oricorio_if_useful
#undef bench_from_hand
#define attach_manual attach_manual_tate_blender_original
#include "trace_engine_v2/part_007.inc"
#undef attach_manual
#include "trace_engine_v2/part_tate_blender_attachment_override.inc"
#define play_tate_switch play_tate_switch_tate_blender_original
#define play_tate_draw play_tate_draw_tate_blender_original
#include "trace_engine_v2/part_008a.inc"
#undef play_tate_draw
#undef play_tate_switch
#include "trace_engine_v2/part_tate_blender_tate_override.inc"
#define play_mysterious_treasure play_mysterious_treasure_empty_deck_original
#include "trace_engine_v2/part_008b.inc"
#include "trace_engine_v2/part_009a.inc"
#undef play_mysterious_treasure
#define play_quick_ball play_quick_ball_empty_deck_original
#include "trace_engine_v2/part_009b1.inc"
#undef play_quick_ball
#define play_ultra_ball play_ultra_ball_original
#include "trace_engine_v2/part_009b2.inc"
#define play_evolution_incense play_evolution_incense_original
#define play_earthen_vessel play_earthen_vessel_empty_deck_original
#define play_brilliant_blender play_brilliant_blender_empty_deck_original
#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#include "trace_engine_v2/part_010.inc"
#undef attach_fss
#undef fss_target_after_search_started
#undef play_brilliant_blender
#include "trace_engine_v2/part_010_blender_thinning_override.inc"
#undef play_earthen_vessel
#undef play_evolution_incense
#undef play_ultra_ball
#define bench_tapu_if_useful bench_tapu_if_useful_empty_deck_original
#include "trace_engine_v2/part_tapu_tate_switch_override.inc"
#undef bench_tapu_if_useful
#define play_ultra_ball play_ultra_ball_empty_deck_original
#define play_evolution_incense play_evolution_incense_empty_deck_original
#include "trace_engine_v2/part_search_item_overrides.inc"
#undef play_evolution_incense
#undef play_ultra_ball
#include "trace_engine_v2/part_010_fss_override.inc"
#include "trace_engine_v2/part_010_attach_fss_override.inc"
#define use_fss use_fss_latias_original
#define play_crispin play_crispin_empty_deck_original
#define play_professor_burnet play_professor_burnet_empty_deck_original
#define play_steven play_steven_empty_deck_original
#include "trace_engine_v2/part_011.inc"
#undef play_steven
#undef play_professor_burnet
#include "trace_engine_v2/part_011_burnet_thinning_override.inc"
#undef play_crispin
#define play_arven play_arven_original
#define play_gladion play_gladion_original
#include "trace_engine_v2/part_012.inc"
#undef play_gladion
#undef play_arven
#define use_celestial_roar use_celestial_roar_original
#define use_legacy_star use_legacy_star_original
#include "trace_engine_v2/part_013.inc"
#undef use_legacy_star
#include "trace_engine_v2/part_014a.inc"
#undef use_fss
#define use_fss use_fss_empty_deck_original
#include "trace_engine_v2/part_011_fss_latias_override.inc"
#undef use_fss
#undef use_celestial_roar
#include "trace_engine_v2/part_celestial_roar_override.inc"
#include "trace_engine_v2/part_013_legacy_star_override.inc"
#include "trace_engine_v2/part_012_gladion_override.inc"
#define play_arven play_arven_powerglass_original
#include "trace_engine_v2/part_012_override.inc"
#undef play_arven
#define play_arven play_arven_empty_deck_original
#include "trace_engine_v2/part_012_powerglass_override.inc"
#undef play_arven
#define choose_supporter choose_supporter_original
#include "trace_engine_v2/part_014b.inc"
#undef choose_supporter
#define choose_supporter choose_supporter_turo_original
#include "trace_engine_v2/part_team_yell_vstar_override.inc"
#undef choose_supporter
#include "trace_engine_v2/part_roseanne_multimode_override.inc"
#include "trace_engine_v2/part_turo_oricorio_override.inc"
#include "trace_engine_v2/part_empty_deck_search_override.inc"
#include "trace_engine_v2/part_014c.inc"
#include "trace_engine_v2/part_015.inc"
#include "trace_engine_v2/part_016.inc"

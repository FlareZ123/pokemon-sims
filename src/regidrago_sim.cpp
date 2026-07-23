// Split only for reviewable GitHub contents-API commits. The compiler receives one translation unit.
//
// Direct rules/card-text registry. Method-level mappings and validation notes:
// docs/RULE_SOURCES.md and docs/RULES_TRACEABILITY.md.
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
// Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
// Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
// Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
// Confirmed source-registry fix: https://github.com/FlareZ123/pokemon-sims/issues/856
// Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
// Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
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
#include "trace_engine_v2/part_empty_deck_unseen_override.inc"
#undef begin_turn
#include "trace_engine_v2/part_begin_turn_override.inc"
#include "trace_engine_v2/part_tapu_copy_availability_override.inc"
#define bench_from_hand bench_from_hand_empty_deck_original
#define bench_oricorio_if_useful bench_oricorio_if_useful_empty_deck_original
#define needs_oricorio_connector needs_oricorio_connector_original
#define needs_tapu_connector needs_tapu_connector_original
#define bench_tapu_if_useful bench_tapu_if_useful_original
#define recover_discard_to_hand recover_discard_to_hand_name_only_original
#include "trace_engine_v2/part_006.inc"
#undef recover_discard_to_hand
#undef bench_tapu_if_useful
#undef needs_tapu_connector
#undef needs_oricorio_connector
#undef bench_oricorio_if_useful
#undef bench_from_hand
#define attach_manual attach_manual_tate_blender_original
#include "trace_engine_v2/part_007.inc"
#undef attach_manual
#define play_tate_switch play_tate_switch_tate_blender_original
#define play_tate_draw play_tate_draw_tate_blender_original
#include "trace_engine_v2/part_008a.inc"
#undef play_tate_draw
#undef play_tate_switch
// part_007.inc opens evolve_best_regi(), and part_008a.inc completes it before
// this member-function override may be included:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_007.inc#L169-L172
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_008a.inc#L1-L18
#include "trace_engine_v2/part_discard_recovery_provenance_override.inc"
#include "trace_engine_v2/part_tate_blender_attachment_override.inc"
#include "trace_engine_v2/part_tate_blender_tate_override.inc"
#define play_mysterious_treasure play_mysterious_treasure_empty_deck_original
#define play_heavy_ball play_heavy_ball_prize_payload_original
#include "trace_engine_v2/part_008b.inc"
#undef play_heavy_ball
// Wonder Tag is copy-specific. Preserve all existing selector controls while an
// in-play Tapu no longer suppresses a second physical copy:
// https://api.pokemontcg.io/v2/cards/sm2-60
// https://github.com/FlareZ123/pokemon-sims/issues/746
#define in_play tapu_connector_copy_aware_in_play
#include "trace_engine_v2/part_009a.inc"
#undef in_play
#undef play_mysterious_treasure
#define play_quick_ball play_quick_ball_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#include "trace_engine_v2/part_009b1.inc"
#undef in_play
#undef play_quick_ball
#define play_ultra_ball play_ultra_ball_original
#include "trace_engine_v2/part_009b2.inc"
#define play_evolution_incense play_evolution_incense_original
#define play_earthen_vessel play_earthen_vessel_empty_deck_original
#define play_brilliant_blender play_brilliant_blender_legacy_original
#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#define should_play_steven should_play_steven_original
#include "trace_engine_v2/part_010.inc"
#undef should_play_steven
#undef attach_fss
#undef fss_target_after_search_started
#include "trace_engine_v2/part_010_late_steven_override.inc"
#define should_play_steven should_play_steven_issue1030_original
#include "trace_engine_v2/part_010_steven_crispin_override.inc"
#undef should_play_steven
#undef play_brilliant_blender
// The thinning policy remains the implementation wrapped by the later empty-deck
// guard, while the legacy part_010 implementation stays dormant:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010_blender_thinning_override.inc#L1-L77
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_empty_deck_search_override.inc#L78-L85
#define play_brilliant_blender play_brilliant_blender_empty_deck_original
#include "trace_engine_v2/part_010_blender_thinning_override.inc"
#undef play_brilliant_blender
#undef play_earthen_vessel
#undef play_evolution_incense
#undef play_ultra_ball
#define bench_tapu_if_useful bench_tapu_if_useful_empty_deck_original
#define wonder_tag_crispin_is_redundant_with_held_complete_route \
  wonder_tag_crispin_is_redundant_with_held_complete_route_issue980_original
#define needs_tapu_connector needs_tapu_connector_issue989_original
#define in_play tapu_connector_copy_aware_in_play
#include "trace_engine_v2/part_tapu_tate_switch_override.inc"
#undef in_play
#undef needs_tapu_connector
#undef wonder_tag_crispin_is_redundant_with_held_complete_route
#undef bench_tapu_if_useful
#include "trace_engine_v2/part_issue_989_wonder_tag_complete_route_override.inc"
#define play_ultra_ball play_ultra_ball_empty_deck_original
#define play_evolution_incense play_evolution_incense_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#include "trace_engine_v2/part_search_item_overrides.inc"
#undef in_play
#undef play_evolution_incense
#undef play_ultra_ball
#include "trace_engine_v2/part_pokemon_communication.inc"
#define fss_target_after_search_started fss_target_after_search_started_issue1071_original
#include "trace_engine_v2/part_010_fss_override.inc"
#undef fss_target_after_search_started
#include "trace_engine_v2/part_issue_1071_fss_oricorio_treasure_decomposition_override.inc"
#include "trace_engine_v2/part_010_attach_fss_override.inc"
#define use_fss use_fss_latias_original
#define play_crispin play_crispin_empty_deck_original
#define play_professor_burnet play_professor_burnet_legacy_original
#define play_steven play_steven_empty_deck_original
#include "trace_engine_v2/part_011.inc"
#undef play_steven
#undef play_professor_burnet
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
#define play_professor_burnet play_professor_burnet_empty_deck_original
#include "trace_engine_v2/part_011_burnet_thinning_override.inc"
#undef play_professor_burnet
#undef use_fss
#define use_fss use_fss_empty_deck_original
#include "trace_engine_v2/part_011_fss_latias_override.inc"
#undef use_fss
#undef use_celestial_roar
#define use_celestial_roar use_celestial_roar_issue1369_original
#include "trace_engine_v2/part_celestial_roar_override.inc"
#undef use_celestial_roar
#include "trace_engine_v2/part_legacy_star_projection_provenance_override.inc"
#define remove_one remove_one_legacy_star_projection
#define pokemon_communication_plan pokemon_communication_plan_legacy_star_projection
#define use_legacy_star use_legacy_star_issue1016_original
#include "trace_engine_v2/part_013_legacy_star_override.inc"
#undef use_legacy_star
#undef pokemon_communication_plan
#undef remove_one
#define use_legacy_star use_legacy_star_issue1069_original
#include "trace_engine_v2/part_issue_1016_legacy_star_quick_ball_override.inc"
#undef use_legacy_star
#include "trace_engine_v2/part_issue_1069_legacy_star_combined_energy_payload_override.inc"
#define play_gladion play_gladion_prize_payload_original
#include "trace_engine_v2/part_012_gladion_override.inc"
#undef play_gladion
#define play_gladion play_gladion_issue1030_original
#include "trace_engine_v2/part_prize_payload_outlet_override.inc"
#undef play_gladion
#define play_gladion play_gladion_issue1191_original
#include "trace_engine_v2/part_issue_1030_gladion_turo_override.inc"
#undef play_gladion
#include "trace_engine_v2/part_issue_1191_gladion_steven_override.inc"
#define play_arven play_arven_powerglass_original
#include "trace_engine_v2/part_012_override.inc"
#undef play_arven
#define play_arven play_arven_fss_blender_contention_original
#include "trace_engine_v2/part_012_powerglass_override.inc"
#undef play_arven
#define play_arven play_arven_empty_deck_original
#include "trace_engine_v2/part_012_arven_fss_blender_contention_override.inc"
#undef play_arven
#define choose_supporter choose_supporter_original
#include "trace_engine_v2/part_014b.inc"
#undef choose_supporter
#define choose_supporter choose_supporter_turo_original
#include "trace_engine_v2/part_team_yell_vstar_override.inc"
#undef choose_supporter
#include "trace_engine_v2/part_roseanne_multimode_override.inc"
#define choose_supporter choose_supporter_issue1070_original
#include "trace_engine_v2/part_turo_oricorio_override.inc"
#undef choose_supporter
#define choose_supporter choose_supporter_issue1209_original
#include "trace_engine_v2/part_issue_1070_tate_after_vstar_search_override.inc"
#undef choose_supporter
#define bench_oricorio_if_useful bench_oricorio_if_useful_target_original
#define ultra_ball_has_legal_target ultra_ball_has_legal_target_k0_target_original
#define play_ultra_ball play_ultra_ball_k0_target_original
#define play_brilliant_blender play_brilliant_blender_vstar_axis_original
#define play_earthen_vessel play_earthen_vessel_vstar_window_original
#define play_mysterious_treasure play_mysterious_treasure_issue1167_original
#define play_quick_ball play_quick_ball_issue1209_original
#define bench_tapu_if_useful bench_tapu_if_useful_issue989_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#define play_steven play_steven_issue1002_empty_deck_original
#define use_fss use_fss_issue1356_original
#include "trace_engine_v2/part_empty_deck_search_override.inc"
#undef use_fss
#undef play_steven
#include "trace_engine_v2/part_issue_1356_fss_energy_override.inc"
#undef play_quick_ball
#undef play_mysterious_treasure
#undef in_play
#undef bench_tapu_if_useful
#define play_steven play_steven_issue1030_original
#include "trace_engine_v2/part_issue_1002_steven_target_override.inc"
#undef play_steven
#define should_play_steven should_play_steven_issue1067_original
#include "trace_engine_v2/part_issue_1030_steven_turo_override.inc"
#undef should_play_steven
#include "trace_engine_v2/part_issue_1067_arven_before_late_steven_override.inc"
#define bench_tapu_if_useful bench_tapu_if_useful_issue_991_original
#include "trace_engine_v2/part_issue_989_empty_deck_tapu_override.inc"
#undef bench_tapu_if_useful
#include "trace_engine_v2/part_issue_991_wonder_tag_burnet_legacy_star_override.inc"
#undef play_earthen_vessel
#undef play_brilliant_blender
#undef play_ultra_ball
#undef ultra_ball_has_legal_target
#include "trace_engine_v2/part_k0_ultra_ball_target_override.inc"
#undef bench_oricorio_if_useful
#include "trace_engine_v2/part_oricorio_needed_energy_override.inc"
#include "trace_engine_v2/part_blender_vstar_axis_override.inc"
#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "trace_engine_v2/part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel
#include "trace_engine_v2/part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#define play_mysterious_treasure play_mysterious_treasure_issue1209_original
#include "trace_engine_v2/part_issue_1167_treasure_vessel_override.inc"
#undef play_mysterious_treasure
#define play_quick_ball play_quick_ball_issue1235_original
#define play_mysterious_treasure play_mysterious_treasure_issue1235_original
#define choose_supporter choose_supporter_issue1235_original
#include "trace_engine_v2/part_issue_1209_treasure_tapu_crispin_override.inc"
#undef choose_supporter
#undef play_mysterious_treasure
#undef play_quick_ball
#define play_quick_ball play_quick_ball_issue1356_original
#define play_mysterious_treasure play_mysterious_treasure_issue1356_original
#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"
#undef play_mysterious_treasure
#undef play_quick_ball
#include "trace_engine_v2/part_issue_1356_fss_treasure_energy_override.inc"
#include "trace_engine_v2/part_issue_1118_secret_box.inc"
#include "trace_engine_v2/part_issue_1369_celestial_roar_secret_box_override.inc"
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "trace_engine_v2/part_014c.inc"
#undef run_turn
#include "trace_engine_v2/part_014c_latias_bench_override.inc"
#include "trace_engine_v2/part_015.inc"
#include "trace_engine_v2/part_016.inc"

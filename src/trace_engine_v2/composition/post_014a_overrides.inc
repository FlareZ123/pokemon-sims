#pragma once

// Compose the member overrides that become legal immediately after part_014a.inc
// closes run_search_items_one_step(). Keeping this boundary named avoids exposing
// independent override families in the translation-unit root while preserving
// their established dependency order.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135

#include "../part_issue_1683_serena_dead_quick_ball_override.inc"

// Keep the post-part_014a Burnet and Forest Seal Stone aliases in the same
// composition spine instead of routing them through one-purpose wrapper files.
#if defined(play_professor_burnet)
#error "Post-search Burnet composition requires play_professor_burnet to be undefined on entry"
#endif
#define play_professor_burnet play_professor_burnet_empty_deck_original
#include "../part_011_burnet_thinning_override.inc"
#undef play_professor_burnet
#if defined(play_professor_burnet)
#error "Post-search Burnet composition leaked play_professor_burnet"
#endif

#undef use_fss
#define use_fss use_fss_empty_deck_original
#include "../part_011_fss_latias_override.inc"
#undef use_fss

// Preserve the Celestial Roar alias bridge immediately before the final override
// families, matching the prior wrapper's exact preprocessing lifetime and order.
#undef use_celestial_roar
#define use_celestial_roar use_celestial_roar_issue1369_original
#include "../part_celestial_roar_override.inc"
#undef use_celestial_roar

// Compose Legacy Star projection and payload bridge wrappers directly on the
// post-search spine. Each temporary alias exposes only the preceding implementation.
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
#if defined(remove_one) || defined(pokemon_communication_plan) || defined(use_legacy_star)
#error "Legacy Star projection chain requires wrapper aliases to be undefined on entry"
#endif

#include "../core/legacy_star_projection_provenance.inc" // Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/legacy_star_projection_provenance.inc
#define remove_one remove_one_legacy_star_projection
#define pokemon_communication_plan pokemon_communication_plan_legacy_star_projection
#define use_legacy_star use_legacy_star_issue1016_original
#include "../part_013_legacy_star_override.inc"
#undef use_legacy_star
#undef pokemon_communication_plan
#undef remove_one

#define use_legacy_star use_legacy_star_issue1069_original
#include "../part_issue_1016_legacy_star_quick_ball_override.inc"
#undef use_legacy_star
#include "../part_issue_1069_legacy_star_combined_energy_payload_override.inc"

#if defined(remove_one) || defined(pokemon_communication_plan) || defined(use_legacy_star)
#error "Legacy Star projection chain leaked a wrapper alias"
#endif

// Gladion wrappers live directly on the post-search composition boundary. Each
// alias exposes the implementation from the preceding layer to the next override
// without leaking macro state outward.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
#if defined(play_gladion)
#error "Gladion composition requires play_gladion to be undefined on entry"
#endif

#define play_gladion play_gladion_prize_payload_original
#include "../part_012_gladion_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1030_original
#include "../part_prize_payload_outlet_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1191_original
#include "../part_issue_1030_gladion_turo_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1595_original
#include "../part_issue_1191_gladion_steven_override.inc"
#undef play_gladion

#define play_gladion play_gladion_issue1608_original
#include "../part_issue_1595_gladion_grass_turo_blender_override.inc"
#undef play_gladion

#include "../part_issue_1608_burnet_before_dead_crispin_override.inc"

#if defined(play_gladion)
#error "Gladion composition leaked play_gladion"
#endif

// Arven wrappers live directly on the post-search composition boundary. Each alias
// exposes the implementation from the preceding layer to the next override
// without leaking macro state outward.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
#if defined(play_arven)
#error "Arven composition requires play_arven to be undefined on entry"
#endif

#define play_arven play_arven_powerglass_original
#include "../part_012_override.inc"
#undef play_arven

#define play_arven play_arven_fss_blender_contention_original
#include "../part_012_powerglass_override.inc"
#undef play_arven

#define play_arven play_arven_empty_deck_original
#include "../part_012_arven_fss_blender_contention_override.inc"
#undef play_arven

#if defined(play_arven)
#error "Arven composition leaked play_arven"
#endif

// Keep the late Supporter selector wrapper chain directly on this ordered
// composition boundary. Each alias exposes the preceding implementation to the
// next layer, so define/include/undef order remains behavior-significant.
// Supporter turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
#if defined(choose_supporter)
#error "Supporter selector composition requires choose_supporter to be undefined on entry"
#endif

#define choose_supporter choose_supporter_original
#include "../part_014b.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_turo_original
#include "../part_team_yell_vstar_override.inc"
#undef choose_supporter

#include "../part_roseanne_multimode_override.inc"

#define choose_supporter choose_supporter_klara_original
#include "../part_turo_oricorio_override.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_issue1070_original
#include "../part_klara_recovery_override.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_issue1209_original
#include "../part_issue_1070_tate_after_vstar_search_override.inc"
#undef choose_supporter

#if defined(choose_supporter)
#error "Supporter selector composition leaked choose_supporter"
#endif

// Keep the late-search units directly on their single composition owner. This
// preserves the historical order while removing aggregate and one-purpose wrappers.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#include "empty_deck_search_routes.inc"
#include "post_search_connector_routes.inc"

// Keep the Quick Ball tail on this same late-search spine. Each alias exposes the
// immediately preceding implementation and is released before the next layer.
// Quick Ball card data: https://api.pokemontcg.io/v2/cards/swsh1-179
#if defined(play_quick_ball)
#error "Quick Ball route composition requires play_quick_ball to be undefined on entry"
#endif

#define play_quick_ball play_quick_ball_issue1516_original
#include "../part_issue_1476_redundant_burnet_route_override.inc"
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1595_original
#include "../part_issue_1516_quick_ball_tapu_crispin_override.inc"
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue2272_original
#include "../part_issue_1595_preserve_turo_over_quick_ball_override.inc"
#undef play_quick_ball

// Keep the historical final wrapper compiled under a dormant name so its route
// admission helpers remain available. The active final member is supplied by the
// card-class bridge immediately afterward.
#define play_quick_ball play_quick_ball_card_class_tail_legacy
#include "../part_issue_2272_route_replaced_arven_quick_ball_override.inc"
#undef play_quick_ball
#include "../core/quick_ball_card_class_tail.inc"

#if defined(play_quick_ball)
#error "Quick Ball route composition leaked play_quick_ball"
#endif

// Keep the Crispin provenance shim adjacent to the Secret Box implementation it
// wraps, followed by the Celestial Roar bridge that consumes that implementation.
// Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
// Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
// Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
#if defined(trace)
#error "Secret Box trace composition requires trace to be undefined on entry"
#endif

#include "../core/crispin_trace_provenance.inc" // Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/crispin_trace_provenance.inc
#define trace trace_issue_1437_crispin_provenance
#include "../part_issue_1118_secret_box.inc"
#undef trace
#include "../part_issue_1369_celestial_roar_secret_box_override.inc"

#if defined(trace)
#error "Secret Box trace composition leaked trace"
#endif

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
#include "../core/routes/issue_1016_legacy_star_quick_ball_policy.inc" // Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_1016_legacy_star_quick_ball_policy.inc
#undef use_legacy_star
// Public-projection isolation now lives at the canonical post-search composition boundary.
// #3199 public-projection isolation wrapper. The implementation remains byte-for-byte
// in the adjacent canonical base include; this boundary only prevents Tate's held-route
// shadow simulation from resolving Legacy Star against hidden future deck order.
// Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
// Advanced procedure source: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
// Confirmed semantic bug: https://github.com/FlareZ123/pokemon-sims/issues/3199
#define use_legacy_star use_legacy_star_issue3199_public_projection_original
#include "../core/routes/issue_3199_legacy_star_combined_energy_payload_base.inc"
#undef use_legacy_star

  inline static thread_local int issue_3199_public_projection_depth_{0};

  bool use_legacy_star() {
    if (issue_3199_public_projection_depth_ > 0) return false;
    return use_legacy_star_issue3199_public_projection_original();
  }

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

// Keep the Powerglass continuation at the sole post-search Arven owner. This is
// the former part_012_powerglass_override.inc body at the same macro boundary.
#define play_arven play_arven_fss_blender_contention_original
#include "../core/regidrago_line_helpers.inc"

  bool arven_powerglass_tool_route_live() const {
    if (!state_.active || state_.active->tool != Tool::None ||
        !is_regidrago_v_line(state_.active->card)) {
      return false;
    }

    // Arven consumes the turn's Supporter play before the searched Tool can resolve.
    // When Energy is the only missing axis, Forest Seal Stone can advance that axis
    // after Arven only while the manual attachment remains unused. Powerglass can
    // still attach the needed Basic Energy from discard at end of turn:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool forest_seal_route_available = !state_.vstar_power_used &&
        (hand_count(Card::ForestSealStone) > 0 || might_be_unseen(Card::ForestSealStone));
    const bool energy_is_only_missing_axis = need_energy() && !need_regi() && !need_vstar() &&
        !need_active_vstar() && !need_payload();
    const bool forest_seal_can_advance_after_arven =
        !energy_is_only_missing_axis || !state_.manual_energy_used;
    if ((forest_seal_route_available && forest_seal_can_advance_after_arven) ||
        !might_be_unseen(Card::Powerglass)) {
      return false;
    }

    const bool needed_grass_in_discard =
        state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard =
        state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0;
    return needed_grass_in_discard || needed_fire_in_discard;
  }

  bool play_arven() {
    if (!supporter_allowed() || hand_count(Card::Arven) == 0 ||
        !arven_powerglass_tool_route_live()) {
      return play_arven_powerglass_original();
    }

    // Arven may search for a Pokémon Tool, and Powerglass remains a Pokémon Tool
    // rather than an Item under the Tool erratum, so this route is legal through
    // Item lock. Powerglass then checks its Active holder at end of turn:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven");
    const bool found = move_deck_to_hand(Card::Powerglass);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; R-POWERGLASS-01",
          found ? "Searched Powerglass as the live Pokémon Tool target."
                : "Powerglass was not in the deck after legal search.");
    return true;
  }
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
#include "../core/recovery_supporter_policy.inc" // Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/recovery_supporter_policy.inc
#undef choose_supporter

#define choose_supporter choose_supporter_turo_original
#include "../part_team_yell_vstar_override.inc"
#undef choose_supporter

#include "../part_roseanne_multimode_override.inc"

#define choose_supporter choose_supporter_klara_original
// Tate/Turo public-projection guard now shares the canonical post-search owner.
// #3199 isolates only Legacy Star while Tate's public held-route classifier runs.
// Forest Seal Stone and every other legal deterministic route remain available to
// the projected Engine. A thread-local depth is used because the classifier creates
// a second Engine on the same worker thread; RAII restores the guard on exceptions.
// Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
// Confirmed semantic bug: https://github.com/FlareZ123/pokemon-sims/issues/3199
#define tate_draw_has_held_non_supporter_completion \
  tate_draw_has_held_non_supporter_completion_issue3199_original
#include "../core/routes/issue_3199_turo_oricorio_base.inc"
#undef tate_draw_has_held_non_supporter_completion

  struct PublicProjectionDepthGuard {
    int& depth;

    explicit PublicProjectionDepthGuard(int& value) : depth(value) { ++depth; }
    ~PublicProjectionDepthGuard() { --depth; }

    PublicProjectionDepthGuard(const PublicProjectionDepthGuard&) = delete;
    PublicProjectionDepthGuard& operator=(const PublicProjectionDepthGuard&) = delete;
  };

  bool tate_draw_has_held_non_supporter_completion() const {
    PublicProjectionDepthGuard guard(issue_3199_public_projection_depth_);
    return tate_draw_has_held_non_supporter_completion_issue3199_original();
  }
#undef choose_supporter

#define choose_supporter choose_supporter_issue1070_original
#include "../part_klara_recovery_override.inc"
#undef choose_supporter

#define choose_supporter choose_supporter_issue1209_original
#include "../core/routes/tate_after_vstar_search_selector.inc" // Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/tate_after_vstar_search_selector.inc
#undef choose_supporter

#if defined(choose_supporter)
#error "Supporter selector composition leaked choose_supporter"
#endif

// Keep the late-search units directly on their single composition owner. The
// empty-deck search wrapper stack is inlined here so its alias lifetime is visible
// beside the post-search connector and Quick Ball chains it feeds. Textual order is
// unchanged from the former one-purpose include.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss) || \
    defined(should_play_steven) || defined(play_arven)
#error "Empty-deck search composition requires wrapper aliases to be undefined on entry"
#endif

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
#define play_arven play_arven_garbodor_original
#include "../part_empty_deck_search_override.inc"

#undef play_arven
#if defined(play_arven)
#error "Arven wrapper alias must be released before the Garbodor continuation"
#endif

// The Garbodor/Field Blower Arven continuation has one live composition site, so
// keep its implementation here beside the alias release that makes play_arven final.
  bool arven_garbodor_field_blower_route_live() {
    if (!garbodor_abilities_locked() || item_locked() || !supporter_allowed() ||
        hand_count(Card::Arven) == 0 || hand_count(Card::FieldBlower) > 0 ||
        !deck_search_available() || !might_be_unseen(Card::FieldBlower)) {
      return false;
    }

    // Arven consumes this turn's Supporter action before the searched Item can be
    // played. Therefore Wonder Tag alone cannot justify fetching Field Blower here;
    // require a live non-Supporter Pokemon-Ability route such as Dark Asset, Vital
    // Dance, Skyliner, or Legacy Star:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return garbodor_removal_unlocks_live_pokemon_route(false);
  }

  bool play_arven() {
    if (!arven_garbodor_field_blower_route_live()) {
      return play_arven_garbodor_original();
    }

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven Garbodor Field Blower route");

    std::vector<Card> found;
    if (move_deck_to_hand(Card::FieldBlower)) found.push_back(Card::FieldBlower);

    // Arven may also search a Pokemon Tool. Preserve the established Forest Seal
    // Stone role when Star Alchemy is unused and a Pokemon V has an open Tool slot.
    // Garbotoxin suppresses Pokemon Abilities; Star Alchemy remains the Tool's own
    // Ability under the repository's existing source-bound ruling:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011.inc
    const bool open_v_tool_slot =
        (state_.active && is_pokemon_v(state_.active->card) &&
         state_.active->tool == Tool::None) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
        });
    if (!state_.vstar_power_used && open_v_tool_slot &&
        move_deck_to_hand(Card::ForestSealStone)) {
      found.push_back(Card::ForestSealStone);
    }

    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-CONNECTOR-01",
          "Arven searched the live Garbotoxin answer: " + join_cards(found) + ".");
    return true;
  }

#undef use_fss
#undef play_steven
#if defined(use_fss) || defined(play_steven)
#error "FSS and Steven aliases must be released before the FSS-energy continuation"
#endif
#include "../part_issue_1356_fss_energy_override.inc"

#undef play_quick_ball
#undef play_mysterious_treasure
#undef in_play
#undef bench_tapu_if_useful

#if defined(play_steven) || defined(should_play_steven) || \
    defined(bench_tapu_if_useful)
#error "Late empty-deck selector aliases must be undefined before reuse"
#endif

#define play_steven play_steven_issue1030_original
#include "../part_issue_1002_steven_target_override.inc"
#undef play_steven

#define should_play_steven should_play_steven_issue1067_original
#include "../part_issue_1030_steven_turo_override.inc"
#undef should_play_steven

#define should_play_steven should_play_steven_issue3197_original
#define TRACE_ENGINE_FINAL_PLAY_STEVEN_ALIAS play_steven_issue3197_original
#include "../part_issue_1067_arven_before_late_steven_override.inc"
#undef TRACE_ENGINE_FINAL_PLAY_STEVEN_ALIAS
#undef play_steven
#undef should_play_steven
#include "../part_issue_3197_steven_treasure_crispin_semantic_override.inc"

#define bench_tapu_if_useful bench_tapu_if_useful_issue_991_original
#include "../part_issue_989_empty_deck_tapu_override.inc"
#undef bench_tapu_if_useful

#include "../part_issue_991_wonder_tag_burnet_legacy_star_override.inc"

#undef play_earthen_vessel
#undef play_brilliant_blender
#undef play_ultra_ball
#undef ultra_ball_has_legal_target
#undef bench_oricorio_if_useful

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss) || \
    defined(should_play_steven) || defined(play_arven)
#error "Empty-deck search composition leaked a wrapper alias"
#endif

#include "../core/ultra_ball_target_policy.inc"
#include "../part_blender_vstar_axis_override.inc"
#include "../core/routes/oricorio_connector_policy.inc" // Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/oricorio_connector_policy.inc

// Post-search Earthen Vessel and Mysterious Treasure connector stack. This is the
// sole composition owner, so the wrapper aliases and their release points are kept
// visible beside the preceding empty-deck stack and following Quick Ball tail.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(choose_supporter) || defined(in_play)
#error "Post-search connector composition requires aliases to be undefined on entry"
#endif

#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "../part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel

#define play_earthen_vessel play_earthen_vessel_issue1412_original
#include "../part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#undef play_earthen_vessel

#include "../part_issue_1412_preserve_quick_balls.inc"

#define play_mysterious_treasure play_mysterious_treasure_issue1209_original
#include "../part_issue_1167_treasure_vessel_override.inc"
#undef play_mysterious_treasure

#define play_quick_ball play_quick_ball_issue1235_original
#define play_mysterious_treasure play_mysterious_treasure_issue1235_original
#define choose_supporter choose_supporter_issue1235_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_issue_1209_treasure_tapu_crispin_override.inc"
#undef in_play
#undef choose_supporter
#undef play_mysterious_treasure
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1356_original
#define play_mysterious_treasure play_mysterious_treasure_issue1356_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_issue_1235_oricorio_treasure_tapu_override.inc"
#undef in_play
#undef play_mysterious_treasure
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1476_original
#define play_mysterious_treasure play_mysterious_treasure_issue1476_original
#include "../part_issue_1356_fss_treasure_energy_override.inc"
#undef play_mysterious_treasure
#undef play_quick_ball

#if defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(choose_supporter) || defined(in_play)
#error "Post-search connector composition leaked a wrapper alias"
#endif

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

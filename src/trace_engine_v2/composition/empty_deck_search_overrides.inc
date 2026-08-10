#pragma once

// Keep the empty-deck search wrapper stack in one ordered composition unit.
// Macro aliases deliberately expose the preceding implementation to later wrappers,
// so preserve both include order and each matching undef boundary.
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

  bool arven_garbodor_field_blower_route_live() {
    if (!garbodor_abilities_locked() || item_locked() || !supporter_allowed() ||
        hand_count(Card::Arven) == 0 || hand_count(Card::FieldBlower) > 0 ||
        !deck_search_available() || !might_be_unseen(Card::FieldBlower)) {
      return false;
    }

    // Arven may search one Item. Search Field Blower only when removing Garbodor's
    // attached Tool unlocks a live Pokemon-Ability setup route, preserving Field
    // Blower's matchup-dependent discrete value and the Supporter action:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return garbodor_removal_unlocks_live_pokemon_route();
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

    // Arven may also search one Pokemon Tool. Preserve the established Forest Seal
    // Stone route when Star Alchemy is unused and a Pokemon V has an open Tool slot.
    // Star Alchemy is the Tool's Ability under the repository's existing ruling:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
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
#include "../part_issue_1356_fss_energy_override.inc"

#undef play_quick_ball
#undef play_mysterious_treasure
#undef in_play
#undef bench_tapu_if_useful

#define play_steven play_steven_issue1030_original
#include "../part_issue_1002_steven_target_override.inc"
#undef play_steven

#define should_play_steven should_play_steven_issue1067_original
#include "../part_issue_1030_steven_turo_override.inc"
#undef should_play_steven

#include "../part_issue_1067_arven_before_late_steven_override.inc"

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

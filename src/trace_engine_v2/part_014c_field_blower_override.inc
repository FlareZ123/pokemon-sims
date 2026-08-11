#include "part_issue_1871_treasure_serena_payload_route.inc"
#include "part_issue_1872_treasure_vessel_payload_route.inc"
#include "part_issue_1874_duplicate_treasure_payload_route.inc"
#include "part_issue_1877_treasure_quick_ball_payload_bridge.inc"
#include "part_issue_1878_vessel_quick_ball_tapu_crispin_route.inc"
#include "part_issue_1895_quick_ball_payload_route.inc"
#include "part_issue_1875_quick_ball_tapu_crispin_route.inc"

  bool path_removal_unlocks_live_rule_box_route() {
    if (!rule_box_abilities_locked()) return false;

    // Evaluate Wonder Tag and an already-established Skyliner retreat line in the
    // exact counterfactual state after Path to the Peak leaves play. Restore the
    // public lock state before making the real Item decision:
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/issues/791
    const bool path_lock_removed_before = state_.path_lock_removed;
    state_.path_lock_removed = true;
    const bool wonder_tag_route = supporter_allowed() && hand_count(Card::TapuLeleGX) > 0 &&
        !in_play(Card::TapuLeleGX) && bench_space() > 0 && needs_tapu_connector();
    const bool in_play_skyliner_route = need_active_vstar() &&
        can_free_retreat_with_latias();
    state_.path_lock_removed = path_lock_removed_before;

    // A held Latias ex plus a held VSTAR can preserve the final promotion axis
    // when a prior-turn Benched Regidrago V is ready to evolve. Path suppresses
    // Skyliner until it is removed:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool held_skyliner_route = hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 && !in_play(Card::LatiasEx) &&
        bench_space() > 0 && !state_.retreat_used && state_.active &&
        is_basic(state_.active->card) && state_.active->card != Card::RegidragoV &&
        prior_turn_benched_regi;

    // Legacy Star is a Rule Box Pokémon Ability. Remove Path when an unused VSTAR
    // Power is already in play, or when a prior-turn Regidrago V has the held VSTAR
    // needed to evolve this turn while setup remains unresolved. The exact Legacy
    // Star selector still decides whether its recovery or discard-seven route is
    // worth using after Path leaves play:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool unresolved_setup = setup_axis_missing();
    const bool in_play_legacy_star_route = !state_.vstar_power_used && has_vstar() &&
        unresolved_setup;
    const bool prior_turn_active_regi = active_is_regi_v() && state_.active &&
        state_.active->entered_turn < state_.turn;
    const bool future_legacy_star_route = !state_.vstar_power_used &&
        prior_turn_active_regi && unresolved_setup &&
        hand_count(Card::RegidragoVstar) > 0;

    return wonder_tag_route || in_play_skyliner_route || held_skyliner_route ||
           in_play_legacy_star_route || future_legacy_star_route;
  }

  bool garbodor_removal_unlocks_live_pokemon_route(const bool allow_wonder_tag = true) {
    if (!garbodor_abilities_locked()) return false;

    // Project the exact state after Field Blower removes Garbodor's Tool. Garbotoxin
    // suppresses every Pokemon Ability, so unlike Path this includes non-Rule-Box
    // connectors such as Vital Dance. Restore the lock marker before the real play:
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    const bool lock_removed_before = state_.path_lock_removed;
    state_.path_lock_removed = true;

    // Wonder Tag is useful only while a Supporter can still be played this turn.
    // This also prevents Arven from valuing Field Blower solely for Wonder Tag after
    // Arven itself has consumed the Supporter action:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sv1-166
    const bool wonder_tag_route = allow_wonder_tag && supporter_allowed() &&
        hand_count(Card::TapuLeleGX) > 0 && !in_play(Card::TapuLeleGX) &&
        bench_space() > 0 && needs_tapu_connector();
    const bool in_play_skyliner_route = need_active_vstar() &&
        can_free_retreat_with_latias();
    const bool held_vital_dance_route = hand_count(Card::Oricorio) > 0 &&
        !in_play(Card::Oricorio) && bench_space() > 0 && !state_.deck.empty() &&
        needs_oricorio_connector();
    // Dark Asset checks hand size after Crobat V moves from hand to the Bench. A
    // six-card pre-play hand becomes five, so the Ability can still draw one card:
    // https://api.pokemontcg.io/v2/cards/swsh3-104
    const bool held_dark_asset_route = hand_count(Card::CrobatV) > 0 &&
        !in_play(Card::CrobatV) && bench_space() > 0 && !state_.dark_asset_used &&
        state_.hand.size() <= 6U && setup_axis_missing();

    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool held_skyliner_route = hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 && !in_play(Card::LatiasEx) &&
        bench_space() > 0 && !state_.retreat_used && state_.active &&
        is_basic(state_.active->card) && state_.active->card != Card::RegidragoV &&
        prior_turn_benched_regi;

    const bool unresolved_setup = setup_axis_missing();
    const bool in_play_legacy_star_route = !state_.vstar_power_used && has_vstar() &&
        unresolved_setup;
    const bool prior_turn_active_regi = active_is_regi_v() && state_.active &&
        state_.active->entered_turn < state_.turn;
    const bool future_legacy_star_route = !state_.vstar_power_used &&
        prior_turn_active_regi && unresolved_setup &&
        hand_count(Card::RegidragoVstar) > 0;

    state_.path_lock_removed = lock_removed_before;
    return wonder_tag_route || in_play_skyliner_route || held_skyliner_route ||
           held_vital_dance_route || held_dark_asset_route ||
           in_play_legacy_star_route || future_legacy_star_route;
  }

  Pokemon* spent_forest_seal_stone_holder() {
    if (!state_.vstar_power_used) return nullptr;
    if (state_.active && state_.active->tool == Tool::ForestSealStone) {
      return &*state_.active;
    }
    for (Pokemon& pokemon : state_.bench) {
      if (pokemon.tool == Tool::ForestSealStone) return &pokemon;
    }
    return nullptr;
  }

  bool spent_fss_removal_unlocks_powerglass_route() const {
    if (!state_.vstar_power_used || hand_count(Card::Powerglass) == 0 ||
        !state_.active || state_.active->tool != Tool::ForestSealStone ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar)) {
      return false;
    }

    const bool grass_route = state_.active->grass < 2 &&
        count_of(state_.discard, Card::Grass) > 0;
    const bool fire_route = state_.active->fire < 1 &&
        count_of(state_.discard, Card::Fire) > 0;
    return grass_route || fire_route;
  }

  bool play_field_blower() {
    if (item_locked() || hand_count(Card::FieldBlower) == 0) return false;

    const bool remove_path = rule_box_abilities_locked() &&
        path_removal_unlocks_live_rule_box_route();
    const bool remove_garbodor_tool = garbodor_abilities_locked() &&
        garbodor_removal_unlocks_live_pokemon_route();
    Pokemon* spent_fss = spent_forest_seal_stone_holder();
    const bool clear_active_tool_for_powerglass =
        spent_fss_removal_unlocks_powerglass_route();
    if (!remove_path && !remove_garbodor_tool && !clear_active_tool_for_powerglass) {
      return false;
    }

    // Field Blower can choose up to two Tool/Stadium cards in play, including the
    // player's own. In the Garbodor scenario one target is the Tool maintaining
    // Garbotoxin. Maximum-pressure modeling assumes the opponent has another Tool
    // ready before our next turn, so this Ability unlock is current-turn only:
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    // https://github.com/FlareZ123/pokemon-sims/issues/2807
    remove_one(state_.hand, Card::FieldBlower);
    state_.discard.push_back(Card::FieldBlower);

    if (remove_path || remove_garbodor_tool) state_.path_lock_removed = true;
    if (spent_fss != nullptr &&
        (remove_path || remove_garbodor_tool || clear_active_tool_for_powerglass)) {
      spent_fss->tool = Tool::None;
      state_.discard.push_back(Card::ForestSealStone);
    }

    if (remove_garbodor_tool && spent_fss != nullptr) {
      trace("PLAY ITEM", "R-GAME-ITEM",
            "Field Blower discarded Garbodor's attached Pokemon Tool and a spent Forest Seal Stone; Pokemon Abilities are available for the remainder of this turn.");
    } else if (remove_garbodor_tool) {
      trace("PLAY ITEM", "R-GAME-ITEM",
            "Field Blower discarded Garbodor's attached Pokemon Tool; Pokemon Abilities are available for the remainder of this turn.");
    } else if (remove_path && spent_fss != nullptr) {
      trace("PLAY ITEM", "R-GAME-ITEM",
            "Field Blower discarded the modeled Path to the Peak and a spent Forest Seal Stone.");
    } else if (remove_path) {
      trace("PLAY ITEM", "R-GAME-ITEM",
            "Field Blower discarded the modeled Path to the Peak; Rule Box Pokémon Abilities are available again.");
    } else {
      trace("PLAY ITEM", "R-GAME-ITEM",
            "Field Blower discarded a spent Forest Seal Stone, clearing the Active Pokémon's Tool slot.");
    }
    return true;
  }

  bool secret_box_combo_enabled_issue1875() const {
    return issue_1871_treasure_serena_payload_route_available() ||
        issue_1895_held_crispin_quick_ball_route_available() ||
        issue_1872_treasure_vessel_payload_route_available() ||
        issue_1874_duplicate_treasure_payload_route_available() ||
        issue_1877_treasure_quick_ball_payload_bridge_available() ||
        issue_1878_vessel_quick_ball_tapu_crispin_route_available() ||
        issue_1875_quick_ball_tapu_crispin_route_available() ||
        secret_box_combo_enabled();
  }

  void run_secret_box_turn_issue1875() {
    // Run the deterministic Treasure-to-Serena payload line before Legacy Star or
    // generic Item order can replace a certain public-K1 finish with a random route.
    // https://github.com/FlareZ123/pokemon-sims/issues/1871
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_1871_treasure_serena_payload_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1871_treasure_serena_payload_route()) {
        throw std::logic_error("Issue-1871 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // Run the public-K1 Quick Ball payload route before generic action ordering can
    // consume the held Crispin or generic DCI can preserve the held Dragon:
    // https://github.com/FlareZ123/pokemon-sims/issues/1895
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_1895_held_crispin_quick_ball_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1895_held_crispin_quick_ball_route()) {
        throw std::logic_error("Issue-1895 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // Search the current-turn Dragon before generic Earthen Vessel ordering spends
    // the only safe Treasure cost and loses the deterministic T4 payload channel.
    // https://github.com/FlareZ123/pokemon-sims/issues/1872
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_1872_treasure_vessel_payload_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1872_treasure_vessel_payload_route()) {
        throw std::logic_error("Issue-1872 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // Preserve the duplicate Treasures before generic Earthen Vessel ordering can
    // spend one copy and destroy the deterministic current-turn payload bridge.
    // https://github.com/FlareZ123/pokemon-sims/issues/1874
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_1874_duplicate_treasure_payload_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1874_duplicate_treasure_payload_route()) {
        throw std::logic_error("Issue-1874 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // GGF is complete in this route. Preserve Earthen Vessel as Treasure's safe
    // cost, then use Quick Ball to put the searched Dragon into discard this turn.
    // https://github.com/FlareZ123/pokemon-sims/issues/1877
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_1877_treasure_quick_ball_payload_bridge_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1877_treasure_quick_ball_payload_bridge()) {
        throw std::logic_error("Issue-1877 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    // Run the confirmed complete route before generic action ordering can spend
    // its Dragon payload on Earthen Vessel and strand the Quick Ball continuation.
    // https://github.com/FlareZ123/pokemon-sims/issues/1878
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_1878_vessel_quick_ball_tapu_crispin_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1878_vessel_quick_ball_tapu_crispin_route()) {
        throw std::logic_error("Issue-1878 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    if (issue_1875_quick_ball_tapu_crispin_route_available()) {
      trace("POLICY", "P-AXIS-01", "Start: " + state_line());
      if (!complete_issue_1875_quick_ball_tapu_crispin_route()) {
        throw std::logic_error("Issue-1875 projected route failed");
      }
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
    run_secret_box_turn();
  }

#undef secret_box_combo_enabled
#undef run_secret_box_turn
#define secret_box_combo_enabled secret_box_combo_enabled_issue1875
#define run_secret_box_turn run_secret_box_turn_issue1875

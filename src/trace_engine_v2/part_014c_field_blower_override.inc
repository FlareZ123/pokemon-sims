#include "part_issue_1872_treasure_vessel_payload_route.inc"
#include "part_issue_1874_duplicate_treasure_payload_route.inc"
#include "part_issue_1877_treasure_quick_ball_payload_bridge.inc"
#include "part_issue_1878_vessel_quick_ball_tapu_crispin_route.inc"
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
    const bool wonder_tag_route = hand_count(Card::TapuLeleGX) > 0 &&
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

  bool play_field_blower() {
    if (item_locked() || hand_count(Card::FieldBlower) == 0 ||
        !rule_box_abilities_locked() || !path_removal_unlocks_live_rule_box_route()) {
      return false;
    }

    // Field Blower may discard the Stadium in play. Spend it only when removing
    // the modeled Path to the Peak unlocks a currently live setup route, so a
    // discrete-value Stadium answer is preserved after setup is already complete:
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/issues/791
    remove_one(state_.hand, Card::FieldBlower);
    state_.discard.push_back(Card::FieldBlower);
    state_.path_lock_removed = true;
    trace("PLAY ITEM", "R-GAME-ITEM",
          "Field Blower discarded the modeled Path to the Peak; Rule Box Pokémon Abilities are available again.");
    return true;
  }

  bool secret_box_combo_enabled_issue1875() const {
    return issue_1872_treasure_vessel_payload_route_available() ||
        issue_1874_duplicate_treasure_payload_route_available() ||
        issue_1877_treasure_quick_ball_payload_bridge_available() ||
        issue_1878_vessel_quick_ball_tapu_crispin_route_available() ||
        issue_1875_quick_ball_tapu_crispin_route_available() ||
        secret_box_combo_enabled();
  }

  void run_secret_box_turn_issue1875() {
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

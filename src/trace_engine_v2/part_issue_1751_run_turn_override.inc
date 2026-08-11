  bool issue_1795_crispin_steven_vessel_route_available() {
    const bool held_direct_vstar_connector =
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::EvolutionIncense) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::UltraBall) > 0 ||
        hand_count(Card::PokemonCommunication) > 0 ||
        hand_count(Card::ForestSealStone) > 0 ||
        hand_count(Card::Arven) > 0 || hand_count(Card::SecretBox) > 0;
    const bool held_vessel_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);

    // StrictJit and MatchupFlexJit share the same same-ready-turn payload rule.
    // The route is Trainer-only, so use the actual Supporter and Item legality
    // predicates instead of hard-coding seat, absolute turn, or lock identity:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
    if (!strict_payload_timing() || item_locked() ||
        state_.turn + 2 > scenario_.max_turn || !prizes_known() ||
        !supporter_allowed() || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        !need_energy() || !need_vstar() || !need_payload() ||
        hand_count(Card::Gladion) == 0 || hand_count(Card::Crispin) == 0 ||
        hand_count(Card::StevensResolve) == 0 || !held_vessel_payload ||
        held_direct_vstar_connector ||
        deck_count_after_search_started(Card::Grass) < 2 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0) {
      return false;
    }
    return crispin_can_advance_energy_axis();
  }

  bool issue_1795_banked_steven_route_available() {
    const bool has_or_can_search_vstar =
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
    const bool has_or_can_search_vessel =
        hand_count(Card::EarthenVessel) > 0 ||
        deck_count_after_search_started(Card::EarthenVessel) > 0;
    const bool still_needs_steven = hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::EarthenVessel) == 0;
    const bool has_final_grass = !need_energy() ||
        hand_count(Card::Grass) > 0 ||
        deck_count_after_search_started(Card::Grass) > 0;
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);

    // Preserve the semantic admission after Crispin commits the schedule. Steven
    // and the following Vessel finish remain legal under Rule Box Ability lock;
    // persistent Item lock still invalidates the planned next-turn Vessel:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
    return issue_1795_steven_turn_ == state_.turn &&
        strict_payload_timing() && !item_locked() &&
        state_.turn + 1 <= scenario_.max_turn && prizes_known() &&
        supporter_allowed() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        hand_count(Card::StevensResolve) > 0 && held_payload &&
        has_or_can_search_vstar && has_or_can_search_vessel &&
        still_needs_steven && has_final_grass;
  }

  bool complete_issue_1795_vessel_finish_issue3170() {
    if (issue_1795_vessel_turn_ != state_.turn) return false;

    std::optional<Card> payload;
    for (const Card card : state_.hand) {
      if (is_payload(card)) {
        payload = card;
        break;
      }
    }
    const bool has_final_grass = !need_energy() ||
        hand_count(Card::Grass) > 0 ||
        deck_count_after_search_started(Card::Grass) > 0;

    // Earthen Vessel is the same-ready-turn Dragon outlet in both JIT profiles.
    // Rule Box Ability lock does not disable this Item, evolution, attachment, or
    // attack-cost sequence. Item legality and all actual state requirements stay
    // explicit here rather than being approximated by LockMode::None:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, Item, discard, search, attachment, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
    const bool route_available =
        strict_payload_timing() && !state_.turn_ended && prizes_known() &&
        !state_.manual_energy_used && !item_locked() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        payload.has_value() && has_final_grass;
    if (!route_available) {
      issue_1795_vessel_turn_ = 0;
      return false;
    }

    evolve_best_regi();
    if (!active_is_vstar()) {
      throw std::logic_error("Issue-1795 Active Regidrago V did not evolve");
    }

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*payload, "Earthen Vessel strict-JIT Dragon cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1795 Vessel payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1795 semantic finish");
    std::optional<Card> searched_basic;
    if (need_energy()) {
      const Pokemon* target = target_regi();
      if (target == nullptr) {
        throw std::logic_error("Issue-1795 Energy target disappeared");
      }
      // Vessel searches Basic Energy only. DDE is Special Energy and therefore
      // cannot be a Vessel target: https://api.pokemontcg.io/v2/cards/sv4-163
      // DDE supplies two all-type Energy to Dragon Pokemon: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      const auto held_finish = preferred_manual_energy_from_hand(*target);
      Pokemon projected = *target;
      const bool held_already_finishes = held_finish &&
          attach_energy_card(projected, *held_finish) &&
          pays_apex_energy_cost(projected);
      if (!held_already_finishes) {
        for (const Card basic : {Card::Grass, Card::Fire}) {
          if (deck_count_after_search_started(basic) <= 0) continue;
          Pokemon candidate = *target;
          if (attach_energy_card(candidate, basic) &&
              pays_apex_energy_cost(candidate)) {
            searched_basic = basic;
            break;
          }
        }
        if (!searched_basic || !move_deck_to_hand(*searched_basic)) {
          throw std::logic_error("Issue-1795 final Basic Energy target disappeared");
        }
      }
    }
    shuffle(state_.deck);
    trace("Earthen Vessel", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01",
          searched_basic ?
              "Discarded the held Dragon payload and searched " +
                  std::string(name(*searched_basic)) + " for the final attachment." :
              "Discarded the held Dragon payload; no redundant Basic Energy search was needed.");

    if (need_energy() && !attach_manual()) {
      throw std::logic_error("Issue-1795 final Energy attachment became unavailable");
    }
    issue_1795_vessel_turn_ = 0;
    return active_is_vstar() && pays_apex_energy_cost(*state_.active) &&
        payload_ready();
  }

// Preserve the current #1751 turn policy under a private name, then put the
// semantic #1795 continuation ahead of it. This wrapper changes only the route
// admission boundary; the existing action sequence remains the fallback.
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
#define run_turn run_turn_issue_3170_original
#include "part_issue_1751_run_turn_override_legacy.inc"
#undef run_turn

  void run_turn() {
    if (complete_issue_1795_vessel_finish_issue3170()) {
      trace("POLICY", "R-EV-01; P-JIT-01; P-COMPRESS-01",
            "Completed the semantic issue-1795 Earthen Vessel finish.");
      return;
    }
    run_turn_issue_3170_original();
  }

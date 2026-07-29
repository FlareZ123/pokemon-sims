#include "part_issue_1411_treasure_tapu_steven_override.inc"

  int issue_1417_treasure_hold_turn_ = 0;
  int issue_1844_vessel_hold_turn_ = 0;

  bool issue_1549_mysterious_treasure_target_available() const {
    // At K0, might_be_unseen() uses only the fixed recipe and public zones. After
    // a legal deck or Prize inspection, it uses the exact remaining deck instead:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // K0 and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Hidden-information specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1549
    return std::any_of(
        recipe_.begin(), recipe_.end(), [this](const auto& entry) {
          return entry.second > 0 && is_dragon_or_psychic(entry.first) &&
              might_be_unseen(entry.first);
        });
  }

  bool issue_1356_treasure_vstar_energy_split_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        scenario_.max_turn < 3 || !deck_seen_ || !state_.manual_energy_used ||
        !state_.vstar_power_used || item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool fire_is_next_attachment =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0 && hand_count(Card::Grass) > 0;
    const bool grass_is_next_attachment =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Grass) > 0 && hand_count(Card::Fire) > 0;

    // Star Alchemy has supplied the missing next-turn Energy. The opposite held
    // Energy is now a legal dynamic-DCI cost for Mysterious Treasure, which
    // searches Regidrago VSTAR and preserves a second Treasure plus the held
    // Dragon for the T3 strict-JIT payload:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
    return fire_is_next_attachment || grass_is_next_attachment;
  }

  bool issue_1356_complete_next_turn_route_holds_quick_ball() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        !deck_seen_ || !state_.manual_energy_used || !state_.vstar_power_used ||
        item_locked() || !state_.active ||
        state_.active->entered_turn >= state_.turn ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool fire_is_held_next_attachment =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0;
    const bool grass_is_held_next_attachment =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Grass) > 0;
    const bool vstar_axis_complete = active_is_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool payable_treasure_bridge =
        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        ((fire_is_held_next_attachment && hand_count(Card::Grass) > 0) ||
         (grass_is_held_next_attachment && hand_count(Card::Fire) > 0));

    // Quick Ball into Oricorio spends an Item, a second discard, a support
    // Pokémon, and a Bench slot after the Energy, VSTAR, and payload axes already
    // have complete public routes. It cannot improve the T3 deadline:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Bench, Ability, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Resource-preservation priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1335 https://github.com/FlareZ123/pokemon-sims/issues/1356
    return (vstar_axis_complete || payable_treasure_bridge) &&
        (fire_is_held_next_attachment || grass_is_held_next_attachment);
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1356_complete_next_turn_route_holds_quick_ball()) return false;
    return play_quick_ball_issue1356_original(permit_payload);
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (issue_1817_play_prelock_treasure_tapu_route()) return true;
    if (issue_1411_play_treasure_tapu_steven_route()) return true;
    if (issue_1417_treasure_hold_turn_ == state_.turn) {
      // The recovered Mysterious Treasure is a next-turn connector. Playing it on
      // the Legacy Star turn would discard the held Dragon before GGF is complete,
      // violating the repository strict-JIT timing contract:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Item and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1417
      return false;
    }
    if (!issue_1356_treasure_vstar_energy_split_available()) {
      return play_mysterious_treasure_issue1356_original(permit_payload);
    }

    const Card surplus_energy = state_.active->fire == 0 ? Card::Grass : Card::Fire;
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(surplus_energy, "Mysterious Treasure split-route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1356 surplus Energy cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("projected issue-1356 Regidrago VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded the surplus off-type Energy and searched Regidrago VSTAR; preserved the second Treasure for the T3 Dragon payload.");
    return true;
  }

  bool issue_1417_future_treasure_route_available_before_legacy() const {
    return scenario_.dci == DciProfile::StrictJit && !item_locked() &&
        state_.turn < scenario_.max_turn && active_is_vstar() &&
        state_.manual_energy_used && need_payload() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
        issue_1549_mysterious_treasure_target_available() &&
        ((grass_needed() == 1 && fire_needed() == 0 &&
          hand_count(Card::Grass) > 0) ||
         (fire_needed() == 1 && grass_needed() == 0 &&
          hand_count(Card::Fire) > 0));
  }

  bool use_legacy_star_issue_1417() {
    const bool recover_future_treasure =
        issue_1417_future_treasure_route_available_before_legacy();
    const bool recover_future_vessel =
        issue_1844_future_vessel_route_available_before_legacy();
    const std::size_t hand_before = state_.hand.size();
    const bool used = use_legacy_star();
    if (!used) return false;

    if (recover_future_treasure &&
        state_.hand.size() < hand_before + 2U &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
        issue_1549_mysterious_treasure_target_available()) {
      // Legacy Star permits up to two recovered cards. When its existing policy has
      // used fewer than two slots, recover Mysterious Treasure as the deterministic
      // next-turn payload connector. The held Basic Energy completes GGF by the next
      // manual attachment, and Treasure can then discard the held Dragon on the same
      // strict-JIT ready turn:
      // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
      // Official attachment, Item, search, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest deterministic route and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1417
      if (!recover_discard_to_hand(Card::MysteriousTreasure)) {
        throw std::logic_error("Legacy Star future-turn Treasure target disappeared");
      }
      issue_1417_treasure_hold_turn_ = state_.turn;
      trace("LEGACY STAR ROUTE", "R-VSTAR-01; R-MT-01; P-DCI-01; P-COMPRESS-01",
            "Recovered Mysterious Treasure for the next-turn strict-JIT Dragon payload.");
    }

    if (!recover_future_vessel || state_.hand.size() >= hand_before + 2U ||
        count_of(state_.discard, Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return true;
    }

    const bool exact_needed_energy_remains =
        (grass_needed() == 1 && fire_needed() == 0 &&
         count_of(state_.deck, Card::Grass) > 0) ||
        (fire_needed() == 1 && grass_needed() == 0 &&
         count_of(state_.deck, Card::Fire) > 0);
    if (!exact_needed_energy_remains) return true;

    // Legacy Star may return Earthen Vessel after the seven-card discard. Holding
    // it until the next turn preserves the one-card Item plus one-card payload cost
    // for the same turn that the searched Basic Energy is manually attached:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Item, discard-cost, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    if (!recover_discard_to_hand(Card::EarthenVessel)) {
      throw std::logic_error("Legacy Star future-turn Earthen Vessel target disappeared");
    }
    issue_1844_vessel_hold_turn_ = state_.turn;
    trace("LEGACY STAR ROUTE",
          "R-VSTAR-01; R-EV-01; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Recovered Earthen Vessel for the next-turn strict-JIT Dragon payload and Basic Energy attachment.");
    return true;
  }

  bool issue_1844_future_vessel_route_available_before_legacy() const {
    if (scenario_.dci != DciProfile::StrictJit || item_locked() ||
        !deck_seen_ || state_.turn >= scenario_.max_turn ||
        !active_is_vstar() || !state_.manual_energy_used || !need_payload() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // The next turn has exactly one missing Basic Energy attachment. Requiring the
    // missing Energy to be absent from hand keeps this route distinct from the
    // established Mysterious Treasure continuation, while K1 proves that Vessel
    // will retain the exact legal search target after the top-seven discard:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Official Item, discard-cost, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    return (grass_needed() == 1 && fire_needed() == 0 &&
            hand_count(Card::Grass) == 0 &&
            count_of(state_.deck, Card::Grass) > 0) ||
        (fire_needed() == 1 && grass_needed() == 0 &&
         hand_count(Card::Fire) == 0 &&
         count_of(state_.deck, Card::Fire) > 0);
  }


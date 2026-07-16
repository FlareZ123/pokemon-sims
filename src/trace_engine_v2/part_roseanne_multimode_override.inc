  bool play_roseanne_vstar_energy_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 ||
        !need_vstar() || !need_energy() || state_.manual_energy_used ||
        !has_regidrago_v_that_can_evolve_this_turn() ||
        count_of(state_.discard, Card::RegidragoVstar) == 0) {
      return false;
    }

    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    if (missing_grass + missing_fire != 1) return false;
    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;

    const auto pokemon_communication_then_vessel_is_payable = [this, needed_energy]() {
      if (item_locked() || hand_count(Card::PokemonCommunication) == 0 ||
          hand_count(Card::EarthenVessel) == 0) {
        return false;
      }
      Engine simulated = *this;
      if (!remove_one(simulated.state_.hand, Card::RoseannesBackup)) return false;
      simulated.state_.deck.push_back(Card::RegidragoVstar);
      simulated.state_.deck.push_back(needed_energy);
      const auto plan = simulated.pokemon_communication_plan(false, true, true);
      if (!plan || plan->target != Card::RegidragoVstar ||
          !simulated.resolve_pokemon_communication(plan->returned, plan->target)) {
        return false;
      }
      // Earthen Vessel resolves after the exchange and must retain a distinct legal
      // discard cost in that exact post-Communication hand:
      // https://api.pokemontcg.io/v2/cards/sm9-152
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh9-148
      // https://github.com/FlareZ123/pokemon-sims/issues/701
      return simulated.choose_discard(
          simulated.can_play_payload_this_turn(), true, true,
          Card::EarthenVessel).has_value();
    };

    const auto evolution_incense_then_vessel_is_payable = [this]() {
      if (item_locked() || hand_count(Card::EvolutionIncense) == 0 ||
          hand_count(Card::EarthenVessel) == 0) {
        return false;
      }
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      // Roseanne's Backup and Evolution Incense both leave the hand before Earthen
      // Vessel pays its required discard cost. Test the exact remaining hand so one
      // card cannot be counted as the cost of both searches:
      // https://api.pokemontcg.io/v2/cards/swsh9-148
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sv4-163
      if (!remove_one(state_.hand, Card::RoseannesBackup) ||
          !remove_one(state_.hand, Card::EvolutionIncense)) {
        restore_hand();
        return false;
      }
      // A held Dragon payload may pay Earthen Vessel's other-card cost when that
      // discard establishes the current-turn strict-JIT Apex Dragon payload:
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      const bool payable = choose_discard(can_play_payload_this_turn(), true, true,
                                           Card::EarthenVessel).has_value();
      restore_hand();
      return payable;
    };

    // Roseanne's Backup says "Choose 1 or more," so its Pokémon and Energy
    // modes may resolve together in the same Supporter play:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // Evolution Incense can search the restored Regidrago VSTAR, while Earthen
    // Vessel can search the separately restored Basic Energy for the unused
    // manual attachment:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Play_Basic_Energy_Cards
    if (count_of(state_.discard, needed_energy) == 0 ||
        might_be_unseen(Card::RegidragoVstar) || might_be_unseen(needed_energy) ||
        !(evolution_incense_then_vessel_is_payable() ||
          pokemon_communication_then_vessel_is_payable())) {
      return false;
    }

    remove_one(state_.hand, Card::RoseannesBackup);
    state_.discard.push_back(Card::RoseannesBackup);
    state_.supporter_used = true;

    const auto restore_to_deck = [this](const Card card) {
      if (!remove_one(state_.discard, card)) {
        throw std::logic_error("Roseanne's Backup recovery target disappeared");
      }
      const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                     state_.discarded_this_turn.end(), card);
      if (turn_it != state_.discarded_this_turn.end()) {
        state_.discarded_this_turn.erase(turn_it);
      }
      state_.deck.push_back(card);
    };

    restore_to_deck(Card::RegidragoVstar);
    restore_to_deck(needed_energy);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Roseanne's Backup chose both Pokémon and Energy modes, restoring Regidrago VSTAR and " +
              std::string(name(needed_energy)) +
              " for a payable Pokémon search and Earthen Vessel.");
    return true;
  }

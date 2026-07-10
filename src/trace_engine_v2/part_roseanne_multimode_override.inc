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
        !has_payable_roseanne_vstar_search() || !has_payable_roseanne_energy_vessel()) {
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
              std::string(name(needed_energy)) + " for payable same-turn searches.");
    return true;
  }

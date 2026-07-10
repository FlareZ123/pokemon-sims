  bool turo_oricorio_energy_route_live() const {
    if (!supporter_allowed() || hand_count(Card::ProfessorTuro) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        need_regi() || need_vstar() || need_active_vstar() || need_payload()) {
      return false;
    }

    // This route completes the Active attacker, so compute the deficit from that
    // exact Pokémon rather than from the policy's separately preferred Regidrago:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed + active_fire_needed != 1) return false;

    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;
    const bool needed_energy_might_be_in_deck = might_be_unseen(needed_energy);
    if (!needed_energy_might_be_in_deck) return false;

    // Crispin is the direct Energy Supporter and remains preferred whenever the
    // same exact Basic Energy is searchable. With one type available, the unused
    // manual attachment can still complete the route:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (hand_count(Card::Crispin) > 0) return false;

    return std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::Oricorio;
    });
  }

  void discard_turo_attachment(const Card card) {
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
  }

  bool play_turo_oricorio_energy_route() {
    if (!turo_oricorio_energy_route_live()) return false;

    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;
    auto oricorio = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::Oricorio;
    });
    if (oricorio == state_.bench.end()) return false;

    const Pokemon returned = *oricorio;
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;

    // Professor Turo's Scenario returns the selected Pokémon to hand and discards
    // every attached card: https://api.pokemontcg.io/v2/cards/sv4-171
    for (int energy = 0; energy < returned.grass; ++energy) discard_turo_attachment(Card::Grass);
    for (int energy = 0; energy < returned.fire; ++energy) discard_turo_attachment(Card::Fire);
    if (returned.tool == Tool::ForestSealStone) discard_turo_attachment(Card::ForestSealStone);
    if (returned.tool == Tool::Powerglass) discard_turo_attachment(Card::Powerglass);
    state_.bench.erase(oricorio);
    state_.hand.push_back(Card::Oricorio);

    // Replaying the returned Basic Pokémon onto the open Bench triggers Vital Dance.
    // Search the exact Basic Energy required by the Active Regidrago VSTAR; Vital
    // Dance permits searching for up to 2, so selecting only the required card is legal:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!bench_from_hand(Card::Oricorio, false)) {
      throw std::logic_error("Professor Turo returned Oricorio but its replay failed");
    }
    record_deck_search_knowledge("Vital Dance");
    const bool found = move_deck_to_hand(needed_energy);
    shuffle(state_.deck);
    trace("VITAL DANCE", "R-ORICORIO-01",
          found ? "Searched the exact Basic Energy required by the Active Regidrago VSTAR."
                : "The required Basic Energy was not in the deck after legal search.");
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; R-ORICORIO-01",
          "Professor Turo returned and replayed Oricorio for the Active-specific final Energy route.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed()) return;
    // Evaluate the Active-specific route before the generic preferred-Regidrago axis
    // gate, which may consider a different Benched Regidrago already complete.
    if (play_turo_oricorio_energy_route()) return;
    if (!setup_axis_missing()) return;
    choose_supporter_turo_original();
  }

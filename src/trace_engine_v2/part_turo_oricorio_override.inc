  bool turo_oricorio_energy_route_live() const {
    if (!supporter_allowed() || hand_count(Card::ProfessorTuro) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        need_regi() || need_vstar() || need_active_vstar() || need_payload() ||
        grass_needed() + fire_needed() != 1) {
      return false;
    }

    // Crispin is the direct Energy Supporter and remains the preferred route when it
    // can advance the same setup axis: https://api.pokemontcg.io/v2/cards/sv7-133
    if (hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis()) return false;

    const bool needed_energy_might_be_in_deck =
        (grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
        (fire_needed() == 1 && might_be_unseen(Card::Fire));
    if (!needed_energy_might_be_in_deck) return false;

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

    // Replaying the returned Basic Pokémon onto the open Bench triggers Vital Dance,
    // which may search the final Basic Energy for the unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!bench_from_hand(Card::Oricorio, true)) {
      throw std::logic_error("Professor Turo returned Oricorio but its replay failed");
    }
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; R-ORICORIO-01",
          "Professor Turo returned and replayed Oricorio so Vital Dance could search the final Basic Energy.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (play_turo_oricorio_energy_route()) return;
    choose_supporter_turo_original();
  }

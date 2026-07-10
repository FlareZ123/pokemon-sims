  bool active_powerglass_tool_slot_needed() const {
    if (hand_count(Card::Powerglass) == 0 || !state_.active ||
        state_.active->tool != Tool::None ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar)) {
      return false;
    }

    // Powerglass must be attached to the Pokémon that will be Active when its
    // end-of-turn effect resolves, and that Pokémon may hold only one Tool:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool needed_grass_in_discard =
        state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard =
        state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0;
    return needed_grass_in_discard || needed_fire_in_discard;
  }

  bool attach_fss() {
    if (hand_count(Card::ForestSealStone) == 0) return false;

    // Forest Seal Stone has no Active-Spot requirement. When Powerglass needs the
    // Active Pokémon's only Tool slot, attach the Stone to another eligible Pokémon V:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!state_.vstar_power_used && active_powerglass_tool_slot_needed()) {
      for (Pokemon& pokemon : state_.bench) {
        if (!is_pokemon_v(pokemon.card) || pokemon.tool != Tool::None) continue;
        remove_one(state_.hand, Card::ForestSealStone);
        pokemon.tool = Tool::ForestSealStone;
        trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; R-POWERGLASS-01",
              "Attached Forest Seal Stone to a Benched Pokémon V and reserved the Active Tool slot for Powerglass.");
        return true;
      }
    }

    return attach_fss_original();
  }

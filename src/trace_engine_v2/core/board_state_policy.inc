  template <typename Predicate>
  bool board_has_pokemon_matching(Predicate predicate) const {
    return (state_.active && predicate(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), predicate);
  }

  bool board_has_open_pokemon_v_tool_slot() const {
    const auto open_v = [](const Pokemon& pokemon) {
      return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
    };
    // Pokémon Tools attach to Pokémon in play, and Forest Seal Stone requires a Pokémon V:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    return board_has_pokemon_matching(open_v);
  }

  bool board_has_evolvable_regidrago_v() const {
    const auto can_evolve = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
    };
    // A Basic Pokémon cannot evolve on the turn it was put into play:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Regidrago VSTAR evolves from Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-136
    return (state_.active && can_evolve(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), can_evolve);
  }

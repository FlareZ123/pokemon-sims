  template <typename Predicate>
  [[nodiscard]] const Pokemon* board_first_pokemon_matching(
      Predicate predicate) const {
    // Keep Active-first ordering explicit so every board selector shares the same
    // deterministic traversal before considering Benched Pokémon.
    if (state_.active && predicate(*state_.active)) return &*state_.active;
    const auto match = std::find_if(
        state_.bench.begin(), state_.bench.end(), predicate);
    return match == state_.bench.end() ? nullptr : &*match;
  }

  template <typename Predicate>
  [[nodiscard]] bool board_has_pokemon_matching(Predicate predicate) const {
    return board_first_pokemon_matching(predicate) != nullptr;
  }

  [[nodiscard]] static bool pokemon_has_open_v_tool_slot(
      const Pokemon& pokemon) {
    return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
  }

  [[nodiscard]] bool board_has_open_pokemon_v_tool_slot() const {
    // Pokémon Tools attach to Pokémon in play, and Forest Seal Stone requires a Pokémon V:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    return board_has_pokemon_matching(pokemon_has_open_v_tool_slot);
  }

  [[nodiscard]] bool pokemon_can_evolve_into_regidrago_vstar(
      const Pokemon& pokemon) const {
    // A Basic Pokémon cannot evolve on the turn it was put into play:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Regidrago VSTAR evolves from Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-136
    return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
  }

  [[nodiscard]] bool board_has_evolvable_regidrago_v() const {
    return board_has_pokemon_matching([this](const Pokemon& pokemon) {
      return pokemon_can_evolve_into_regidrago_vstar(pokemon);
    });
  }
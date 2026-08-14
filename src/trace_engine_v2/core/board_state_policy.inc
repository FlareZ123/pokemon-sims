  template <typename ActiveZone, typename BenchZone, typename Predicate>
  [[nodiscard]] static auto find_board_pokemon_matching_in_zones(
      ActiveZone& active, BenchZone& bench, Predicate predicate)
      -> decltype(&*active) {
    // Keep Active-before-Bench traversal in one implementation so mutable and
    // const board queries cannot diverge as the policy layer evolves.
    // C++ find_if semantics: https://eel.is/c++draft/alg.find
    if (active && predicate(*active)) return &*active;
    const auto match = std::find_if(bench.begin(), bench.end(), predicate);
    return match == bench.end() ? nullptr : &*match;
  }

  template <typename Predicate>
  [[nodiscard]] Pokemon* find_board_pokemon_matching(Predicate predicate) {
    return find_board_pokemon_matching_in_zones(
        state_.active, state_.bench, predicate);
  }

  template <typename Predicate>
  [[nodiscard]] const Pokemon* find_board_pokemon_matching(
      Predicate predicate) const {
    return find_board_pokemon_matching_in_zones(
        state_.active, state_.bench, predicate);
  }

  template <typename Predicate>
  [[nodiscard]] bool board_has_pokemon_matching(Predicate predicate) const {
    return find_board_pokemon_matching(predicate) != nullptr;
  }

  [[nodiscard]] static constexpr bool pokemon_has_open_v_tool_slot(
      const Pokemon& pokemon) {
    return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
  }

  [[nodiscard]] bool board_has_open_pokemon_v_tool_slot() const {
    // Pokémon Tools attach to Pokémon in play, and Forest Seal Stone requires a Pokémon V:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    return board_has_pokemon_matching(pokemon_has_open_v_tool_slot);
  }

  [[nodiscard]] static constexpr bool pokemon_can_evolve_into_regidrago_vstar(
      const Pokemon& pokemon, const int current_turn) {
    // A Basic Pokémon cannot evolve on the turn it was put into play:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Regidrago VSTAR evolves from Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-136
    return pokemon.card == Card::RegidragoV && pokemon.entered_turn < current_turn;
  }

  [[nodiscard]] bool board_has_evolvable_regidrago_v() const {
    const int current_turn = state_.turn;
    return board_has_pokemon_matching([current_turn](const Pokemon& pokemon) {
      return pokemon_can_evolve_into_regidrago_vstar(pokemon, current_turn);
    });
  }

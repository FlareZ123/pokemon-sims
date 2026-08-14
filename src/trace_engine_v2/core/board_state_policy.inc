  // Board index 0 is Active; subsequent indexes map to Bench positions. Keep the
  // representation beside the traversal implementation so every board-index user
  // shares one mapping contract.
  // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  using BoardIndex = std::size_t;
  using OptionalBoardIndex = std::optional<BoardIndex>;

  [[nodiscard]] static constexpr BoardIndex board_index_from_bench_index(
      const BoardIndex bench_index) {
    // Canonical board mapping: Active is 0 and Bench positions are offset by one.
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    return bench_index + 1U;
  }

  [[nodiscard]] static constexpr OptionalBoardIndex bench_index_from_board_index(
      const BoardIndex board_index) {
    // Board index 0 denotes Active and therefore has no Bench index.
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    if (board_index == 0U) return std::nullopt;
    return board_index - 1U;
  }

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

  template <typename ActiveZone, typename BenchZone>
  [[nodiscard]] static auto board_pokemon_at_index_in_zones(
      ActiveZone& active, BenchZone& bench, const BoardIndex index)
      -> decltype(&*active) {
    // Centralize the historical Exploding Energy destination mapping before
    // callers migrate away from raw Active/Bench index arithmetic.
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    if (index == 0U) return active ? &*active : nullptr;
    const OptionalBoardIndex bench_index = bench_index_from_board_index(index);
    return bench_index && *bench_index < bench.size()
        ? &bench[*bench_index]
        : nullptr;
  }

  template <typename ActiveZone, typename BenchZone, typename Predicate>
  [[nodiscard]] static OptionalBoardIndex find_board_index_matching_in_zones(
      const ActiveZone& active, const BenchZone& bench, Predicate predicate) {
    // Keep index discovery on the same Active-first mapping used by lookup.
    // C++ container indexing semantics: https://eel.is/c++draft/vector.capacity
    if (active && predicate(*active)) return 0U;
    for (BoardIndex index = 0; index < bench.size(); ++index) {
      if (predicate(bench[index])) return board_index_from_bench_index(index);
    }
    return std::nullopt;
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

  [[nodiscard]] Pokemon* board_pokemon_at_index(const BoardIndex index) {
    return board_pokemon_at_index_in_zones(state_.active, state_.bench, index);
  }

  [[nodiscard]] const Pokemon* board_pokemon_at_index(
      const BoardIndex index) const {
    return board_pokemon_at_index_in_zones(state_.active, state_.bench, index);
  }

  template <typename Predicate>
  [[nodiscard]] OptionalBoardIndex board_index_matching(
      Predicate predicate) const {
    return find_board_index_matching_in_zones(
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

  [[nodiscard]] static constexpr bool pokemon_entered_before_turn(
      const Pokemon& pokemon, const int current_turn) {
    // A Pokémon cannot evolve during the turn it was put into play. Keep the
    // shared prior-turn timing test beside the canonical board traversal seam:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    return pokemon.entered_turn < current_turn;
  }

  [[nodiscard]] static constexpr bool pokemon_can_evolve_into_regidrago_vstar(
      const Pokemon& pokemon, const int current_turn) {
    // Regidrago VSTAR evolves from Regidrago V:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    return pokemon.card == Card::RegidragoV &&
           pokemon_entered_before_turn(pokemon, current_turn);
  }

  [[nodiscard]] bool board_has_evolvable_regidrago_v() const {
    const int current_turn = state_.turn;
    return board_has_pokemon_matching([current_turn](const Pokemon& pokemon) {
      return pokemon_can_evolve_into_regidrago_vstar(pokemon, current_turn);
    });
  }

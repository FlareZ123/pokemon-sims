  // Board index 0 is Active; subsequent indexes map to Bench positions. Keep the
  // representation beside the traversal implementation so every board-index user
  // shares one mapping contract.
  // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  using BoardIndex = std::size_t;
  using OptionalBoardIndex = std::optional<BoardIndex>;
  using BoardAttachmentDestinations = std::vector<BoardIndex>;
  inline static constexpr BoardIndex kActiveBoardIndex = 0U;

  [[nodiscard]] static constexpr bool board_index_is_active(
      const BoardIndex index) {
    return index == kActiveBoardIndex;
  }

  [[nodiscard]] static constexpr BoardIndex board_index_for_bench_position(
      const BoardIndex bench_position) {
    return bench_position + 1U;
  }

  [[nodiscard]] static constexpr BoardIndex bench_position_from_board_index(
      const BoardIndex index) {
    return index - 1U;
  }

  template <typename ActiveZone, typename BenchZone>
  [[nodiscard]] static auto board_pokemon_at_index_in_zones(
      ActiveZone& active, BenchZone& bench, const BoardIndex index)
      -> decltype(&*active) {
    // Centralize the historical Exploding Energy destination mapping before
    // callers migrate away from raw Active/Bench index arithmetic.
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    if (board_index_is_active(index)) return active ? &*active : nullptr;
    const BoardIndex bench_position = bench_position_from_board_index(index);
    return bench_position < bench.size() ? &bench[bench_position] : nullptr;
  }

  template <typename ActiveZone, typename BenchZone, typename Predicate>
  [[nodiscard]] static OptionalBoardIndex board_index_matching_in_zones(
      const ActiveZone& active, const BenchZone& bench, Predicate predicate) {
    // Keep index discovery on the same Active-first mapping used by lookup.
    // C++ container indexing semantics: https://eel.is/c++draft/vector.capacity
    if (active && predicate(*active)) return kActiveBoardIndex;
    for (BoardIndex bench_position = 0; bench_position < bench.size();
         ++bench_position) {
      if (predicate(bench[bench_position])) {
        return board_index_for_bench_position(bench_position);
      }
    }
    return std::nullopt;
  }

  template <typename ActiveZone, typename BenchZone>
  [[nodiscard]] static OptionalBoardIndex board_index_for_pointer_in_zones(
      const ActiveZone& active, const BenchZone& bench, const Pokemon* target) {
    // Pointer lookup reuses the canonical Active-first predicate traversal instead
    // of maintaining a second copy of the board-index loop. Keep the internal name
    // aligned with the public board_index_for_pointer() seam so callers do not need
    // separate "find" and "index" vocabulary for the same operation.
    // C++ pointer equality semantics: https://eel.is/c++draft/expr.eq
    if (target == nullptr) return std::nullopt;
    return board_index_matching_in_zones(
        active, bench,
        [target](const Pokemon& pokemon) { return target == &pokemon; });
  }

  template <typename ActiveZone, typename BenchZone, typename Predicate>
  [[nodiscard]] static auto find_board_pokemon_matching_in_zones(
      ActiveZone& active, BenchZone& bench, Predicate predicate)
      -> decltype(&*active) {
    // Reuse canonical index discovery plus index lookup so every mutable and const
    // board query shares the same Active-before-Bench traversal and index mapping.
    // C++ class-member lookup and find semantics: https://eel.is/c++draft/class.member.lookup
    const OptionalBoardIndex index =
        board_index_matching_in_zones(active, bench, predicate);
    return index ? board_pokemon_at_index_in_zones(active, bench, *index) : nullptr;
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
    return board_index_matching_in_zones(
        state_.active, state_.bench, predicate);
  }

  [[nodiscard]] OptionalBoardIndex board_index_for_pointer(
      const Pokemon* target) const {
    return board_index_for_pointer_in_zones(
        state_.active, state_.bench, target);
  }

  template <typename Predicate>
  [[nodiscard]] bool board_has_pokemon_matching(Predicate predicate) const {
    // Existence queries share canonical Active-first index discovery instead of
    // resolving a pointer when the caller only needs presence information.
    // Canonical board-query owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
    return board_index_matching(predicate).has_value();
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

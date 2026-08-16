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

  [[nodiscard]] static constexpr bool board_index_is_benched(
      const BoardIndex index) {
    return !board_index_is_active(index);
  }

  [[nodiscard]] static constexpr BoardIndex board_index_for_bench_position(
      const BoardIndex bench_position) {
    return bench_position + 1U;
  }

  [[nodiscard]] static constexpr BoardIndex bench_position_from_board_index(
      const BoardIndex index) {
    return index - 1U;
  }

  template <typename ActiveZone, typename BenchZone, typename Visitor>
  [[nodiscard]] static bool visit_board_pokemon_in_zones(
      const ActiveZone& active, const BenchZone& bench, Visitor visitor) {
    if (active && !visitor(kActiveBoardIndex, *active)) return false;
    for (BoardIndex bench_position = 0; bench_position < bench.size();
         ++bench_position) {
      if (!visitor(board_index_for_bench_position(bench_position),
                   bench[bench_position])) {
        return false;
      }
    }
    return true;
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
    OptionalBoardIndex match;
    (void)visit_board_pokemon_in_zones(
        active, bench,
        [&](const BoardIndex index, const Pokemon& pokemon) {
          if (!predicate(pokemon)) return true;
          match = index;
          return false;
        });
    return match;
  }

  template <typename ActiveZone, typename BenchZone, typename Predicate>
  [[nodiscard]] static OptionalBoardIndex benched_pokemon_index_matching_in_zones(
      const ActiveZone& active, const BenchZone& bench, Predicate predicate) {
    // Bench-only route queries use the same board-index mapping and traversal as
    // every other board lookup. This replaces route-local std::find_if/std::any_of
    // scans without changing Bench order or strategy-owned predicates.
    // C++20 traversal semantics: https://eel.is/c++draft/alg.find
    // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    OptionalBoardIndex match;
    (void)visit_board_pokemon_in_zones(
        active, bench,
        [&](const BoardIndex index, const Pokemon& pokemon) {
          if (board_index_is_active(index) || !predicate(pokemon)) return true;
          match = index;
          return false;
        });
    return match;
  }

  template <typename ActiveZone, typename BenchZone, typename Predicate,
            typename Rank>
  [[nodiscard]] static OptionalBoardIndex board_index_best_matching_in_zones(
      const ActiveZone& active, const BenchZone& bench, Predicate predicate,
      Rank rank) {
    // Keep ranked selection on the canonical Active-first traversal. Replacing
    // the best index only for a strictly greater rank preserves first-match ties,
    // matching std::max_element's deterministic ordering for equivalent entries.
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    // max_element tie semantics: https://eel.is/c++draft/alg.sorting#alg.max.min
    OptionalBoardIndex best;
    (void)visit_board_pokemon_in_zones(
        active, bench,
        [&](const BoardIndex index, const Pokemon& pokemon) {
          if (!predicate(index, pokemon)) return true;
          if (!best) {
            best = index;
            return true;
          }
          const Pokemon* current =
              board_pokemon_at_index_in_zones(active, bench, *best);
          if (rank(*current) < rank(pokemon)) best = index;
          return true;
        });
    return best;
  }

  template <typename ActiveZone, typename BenchZone>
  [[nodiscard]] static OptionalBoardIndex board_index_for_pointer_in_zones(
      const ActiveZone& active, const BenchZone& bench, const Pokemon* target) {
    // Pointer lookup reuses the canonical Active-first predicate traversal instead
    // of maintaining a second copy of the board-index loop.
    // C++ pointer equality semantics: https://eel.is/c++draft/expr.eq
    if (target == nullptr) return std::nullopt;
    return board_index_matching_in_zones(
        active, bench,
        [target](const Pokemon& pokemon) { return target == &pokemon; });
  }

  template <typename ActiveZone, typename BenchZone, typename Predicate>
  [[nodiscard]] static auto board_pokemon_matching_in_zones(
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
    return board_pokemon_matching_in_zones(
        state_.active, state_.bench, predicate);
  }

  template <typename Predicate>
  [[nodiscard]] const Pokemon* find_board_pokemon_matching(
      Predicate predicate) const {
    return board_pokemon_matching_in_zones(
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

  template <typename Predicate>
  [[nodiscard]] OptionalBoardIndex benched_pokemon_index_matching(
      Predicate predicate) const {
    return benched_pokemon_index_matching_in_zones(
        state_.active, state_.bench, predicate);
  }

  template <typename Predicate>
  [[nodiscard]] const Pokemon* find_benched_pokemon_matching(
      Predicate predicate) const {
    const OptionalBoardIndex index = benched_pokemon_index_matching(predicate);
    return index ? board_pokemon_at_index(*index) : nullptr;
  }

  template <typename Predicate>
  [[nodiscard]] bool bench_has_pokemon_matching(Predicate predicate) const {
    return benched_pokemon_index_matching(predicate).has_value();
  }

  template <typename Predicate, typename Rank>
  [[nodiscard]] OptionalBoardIndex board_index_best_matching(
      Predicate predicate, Rank rank) const {
    return board_index_best_matching_in_zones(
        state_.active, state_.bench, predicate, rank);
  }

  template <typename Rank>
  [[nodiscard]] OptionalBoardIndex best_benched_pokemon_index(Rank rank) const {
    // Bench ranking is a board-policy operation. Keep the Active exclusion and
    // first-equal tie behavior in one reusable query before Forretress consumes it.
    // Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
    return board_index_best_matching(
        [](const BoardIndex index, const Pokemon&) {
          return board_index_is_benched(index);
        },
        rank);
  }

  [[nodiscard]] OptionalBoardIndex board_index_for_pointer(
      const Pokemon* target) const {
    return board_index_for_pointer_in_zones(
        state_.active, state_.bench, target);
  }

  [[nodiscard]] OptionalBoardIndex board_index_for_card(const Card card) const {
    // Exact-card lookup reuses the canonical Active-first index query so callers
    // do not duplicate card predicates or direct Active/Bench source checks.
    // Canonical board-query owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
    return board_index_matching(
        [card](const Pokemon& pokemon) { return pokemon.card == card; });
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

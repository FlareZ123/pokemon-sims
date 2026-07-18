  bool remove_one_legacy_star_projection(std::vector<Card>& zone, const Card card) {
    const bool projects_discard_recovery = &zone == &state_.discard;
    const int discard_copies = projects_discard_recovery ? count_of(state_.discard, card) : 0;
    const int current_turn_copies = projects_discard_recovery
        ? count_of(state_.discarded_this_turn, card)
        : 0;
    if (!remove_one(zone, card)) return false;

    // Pokémon Communication preflight projects a Legacy Star recovery before it
    // scores the exchange Pokémon. Keep the copied discard and same-turn provenance
    // synchronized with the live recovery transition, including duplicate selection:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/936
    if (projects_discard_recovery && current_turn_copies > 0 &&
        discard_copies == current_turn_copies) {
      const auto turn_it = std::find(
          state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), card);
      state_.discarded_this_turn.erase(turn_it);
    }
    return true;
  }

  bool recover_discard_to_hand(const Card card) {
    const int discard_copies = count_of(state_.discard, card);
    if (discard_copies == 0) return false;
    const int current_turn_copies = count_of(state_.discarded_this_turn, card);

    const auto discard_it = std::find(state_.discard.begin(), state_.discard.end(), card);
    state_.hand.push_back(*discard_it);
    state_.discard.erase(discard_it);

    // When an older copy exists, recovery can select it and preserve the current-turn
    // copy in discard. Remove provenance only when every matching discard copy entered
    // discard this turn:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/934
    if (current_turn_copies > 0 && discard_copies == current_turn_copies) {
      const auto turn_it = std::find(
          state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), card);
      state_.discarded_this_turn.erase(turn_it);
    }
    return true;
  }

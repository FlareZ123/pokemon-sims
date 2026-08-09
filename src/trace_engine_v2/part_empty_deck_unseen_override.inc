  bool deck_has_cards() const {
    return !state_.deck.empty();
  }

  bool inspected_deck_contains(const Card card) const {
    return std::find(state_.deck.begin(), state_.deck.end(), card) != state_.deck.end();
  }

  bool known_deck_contains(const Card card) const {
    // Exact deck membership is actionable only after a legal deck inspection (K1):
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ && inspected_deck_contains(card);
  }

  bool might_be_unseen(const Card card) const {
    // The remaining deck size is public game state. A deck with zero cards cannot
    // contain any hidden candidate, even before a legal deck or Prize inspection:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    return deck_has_cards() && might_be_unseen_empty_deck_original(card);
  }

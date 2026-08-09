  bool deck_has_cards() const {
    return !state_.deck.empty();
  }

  bool known_deck_contains(const Card card) const {
    // Exact deck membership is actionable only after a legal deck inspection (K1):
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ && deck_count_after_search_started(card) > 0;
  }

  bool has_known_hand_or_deck_copy(const Card card) const {
    return hand_count(card) > 0 || known_deck_contains(card);
  }

  bool might_be_unseen(const Card card) const {
    // The remaining deck size is public game state. A deck with zero cards cannot
    // contain any hidden candidate, even before a legal deck or Prize inspection:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!deck_has_cards()) return false;
    return might_be_unseen_empty_deck_original(card);
  }

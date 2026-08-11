  bool deck_has_cards() const {
    return !state_.deck.empty();
  }

  int loose_public_copy_count(const Card card) const {
    return hand_count(card) + count_of(state_.discard, card);
  }

  int publicly_known_copy_count(const Card card) const {
    // K0 decisions may use only cards exposed in public zones, rather than hidden
    // deck-versus-Prize placement:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    return loose_public_copy_count(card) + attached_public_count(card);
  }

  int unresolved_copy_count(const Card card, const int known_copies) const {
    return std::max(0, total_copies(card) - known_copies);
  }

  int unresolved_fixed_list_copy_count(const Card card) const {
    return unresolved_copy_count(card, publicly_known_copy_count(card));
  }

  int known_deck_copy_count(const Card card) const {
    // Exact deck membership is actionable only after a legal deck inspection (K1):
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ ? deck_count_after_search_started(card) : 0;
  }

  bool known_deck_contains(const Card card) const {
    return known_deck_copy_count(card) > 0;
  }

  bool has_known_hand_or_deck_copy(const Card card) const {
    return hand_count(card) > 0 || known_deck_contains(card);
  }
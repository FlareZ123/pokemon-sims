  bool deck_has_cards() const {
    return !state_.deck.empty();
  }

  int publicly_known_copy_count(const Card card) const {
    // K0 decisions may use only cards exposed in public zones, rather than hidden
    // deck-versus-Prize placement:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    return hand_count(card) + count_of(state_.discard, card) +
        attached_public_count(card);
  }

  int unresolved_fixed_list_copy_count(const Card card) const {
    return std::max(0, total_copies(card) - publicly_known_copy_count(card));
  }

  bool known_deck_contains(const Card card) const {
    // Exact deck membership is actionable only after a legal deck inspection (K1):
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ && deck_count_after_search_started(card) > 0;
  }

  bool has_known_hand_or_deck_copy(const Card card) const {
    return hand_count(card) > 0 || known_deck_contains(card);
  }

  [[nodiscard]] bool deck_has_cards() const {
    return !state_.deck.empty();
  }

  [[nodiscard]] int unattached_public_copy_count(const Card card) const {
    // Public-copy aggregation remains source-bounded to zones the player may
    // actually know before K1:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    return KnowledgeCopyPolicy::combined(hand_count(card),
                                         count_of(state_.discard, card));
  }

  [[nodiscard]] int publicly_known_copy_count(const Card card) const {
    // K0 decisions may use only cards exposed in public zones, rather than hidden
    // deck-versus-Prize placement:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    return KnowledgeCopyPolicy::combined(unattached_public_copy_count(card),
                                         attached_public_count(card));
  }

  [[nodiscard]] int unresolved_copy_count(
      const Card card, const int known_copies) const {
    return KnowledgeCopyPolicy::unresolved(total_copies(card), known_copies);
  }

  [[nodiscard]] int unresolved_fixed_list_copy_count(const Card card) const {
    return unresolved_copy_count(card, publicly_known_copy_count(card));
  }

  [[nodiscard]] int known_deck_copy_count(const Card card) const {
    // Exact deck membership is actionable only after a legal deck inspection (K1):
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ ? deck_count_after_search_started(card) : 0;
  }

  [[nodiscard]] bool known_deck_contains(const Card card) const {
    return KnowledgeCopyPolicy::has_any(known_deck_copy_count(card));
  }

  [[nodiscard]] int known_hand_or_deck_copy_count(const Card card) const {
    return KnowledgeCopyPolicy::combined(hand_count(card),
                                         known_deck_copy_count(card));
  }

  [[nodiscard]] bool has_known_hand_or_deck_copy(const Card card) const {
    return KnowledgeCopyPolicy::has_any(known_hand_or_deck_copy_count(card));
  }

  [[nodiscard]] bool might_be_unseen(const Card card) const {
    // The remaining deck size is public game state. A deck with zero cards cannot
    // contain any hidden candidate, even before a legal deck or Prize inspection:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_has_cards() && might_be_unseen_empty_deck_original(card);
  }

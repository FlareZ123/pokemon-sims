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

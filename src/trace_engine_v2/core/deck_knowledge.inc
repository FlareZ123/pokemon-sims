  struct KnowledgeCopyPolicy {
    [[nodiscard]] static constexpr bool has_any(const int count) {
      return count > 0;
    }

    [[nodiscard]] static constexpr int combined(const int first_count,
                                                const int second_count) {
      return first_count + second_count;
    }

    [[nodiscard]] static constexpr int combined_unattached_public_zones(
        const int hand_count, const int discard_count) {
      // Hand and discard are the shared public-zone base used by both unattached
      // K0 accounting and the hand/discard/attached aggregate below.
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#catalog-and-knowledge-cleanup
      return combined(hand_count, discard_count);
    }

    [[nodiscard]] static constexpr int combined_public_zones(
        const int hand_count, const int discard_count, const int attached_count) {
      // Keep public hand/discard/attached aggregation behind one named arithmetic
      // seam after Engine callers have already resolved zone visibility.
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#catalog-and-knowledge-cleanup
      return combined(
          combined_unattached_public_zones(hand_count, discard_count),
          attached_count);
    }

    [[nodiscard]] static int unresolved(const int total_copies,
                                        const int known_copies) {
      return std::max(0, total_copies - known_copies);
    }
  };

  struct PublicZoneCopyCounts {
    int hand = 0;
    int discard = 0;
    int attached = 0;

    [[nodiscard]] constexpr int unattached() const {
      return KnowledgeCopyPolicy::combined_unattached_public_zones(hand, discard);
    }

    [[nodiscard]] constexpr int total() const {
      return KnowledgeCopyPolicy::combined_public_zones(hand, discard, attached);
    }
  };

  struct KnownSearchCopyCounts {
    int hand = 0;
    int deck = 0;

    [[nodiscard]] constexpr int total() const {
      return KnowledgeCopyPolicy::combined(hand, deck);
    }

    [[nodiscard]] constexpr bool has_any() const {
      return KnowledgeCopyPolicy::has_any(total());
    }
  };

  [[nodiscard]] bool deck_has_cards() const {
    return !state_.deck.empty();
  }

  [[nodiscard]] PublicZoneCopyCounts public_unattached_zone_copy_counts(
      const Card card) const {
    // K0 may use copies in public zones without consulting hidden deck/Prize
    // placement. Collect hand/discard once so callers share the same boundary:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    return PublicZoneCopyCounts{
        .hand = hand_count(card),
        .discard = count_of(state_.discard, card),
    };
  }

  [[nodiscard]] PublicZoneCopyCounts public_zone_copy_counts(
      const Card card) const {
    PublicZoneCopyCounts counts = public_unattached_zone_copy_counts(card);
    counts.attached = attached_public_count(card);
    return counts;
  }

  [[nodiscard]] int unattached_public_copy_count(const Card card) const {
    // Public-copy aggregation remains source-bounded to zones the player may
    // actually know before K1:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    return public_unattached_zone_copy_counts(card).unattached();
  }

  [[nodiscard]] int publicly_known_copy_count(const Card card) const {
    // K0 decisions may use only cards exposed in public zones, rather than hidden
    // deck-versus-Prize placement:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    return public_zone_copy_counts(card).total();
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

  [[nodiscard]] KnownSearchCopyCounts known_search_copy_counts(
      const Card card) const {
    // Once K1 is established, hand plus known-deck copies form a common query
    // boundary for route selectors. Keep hidden-deck access behind known_deck_copy_count():
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return KnownSearchCopyCounts{
        .hand = hand_count(card),
        .deck = known_deck_copy_count(card),
    };
  }

  [[nodiscard]] int known_hand_or_deck_copy_count(const Card card) const {
    return known_search_copy_counts(card).total();
  }

  [[nodiscard]] bool has_known_hand_or_deck_copy(const Card card) const {
    return known_search_copy_counts(card).has_any();
  }

  [[nodiscard]] bool might_be_unseen(const Card card) const {
    // The remaining deck size is public game state. A deck with zero cards cannot
    // contain any hidden candidate, even before a legal deck or Prize inspection:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_has_cards() && might_be_unseen_empty_deck_original(card);
  }
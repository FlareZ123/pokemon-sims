  struct PayloadPreferencePolicy {
    inline static constexpr std::array<Card, 5> kOrder{{
        Card::Appletun,
        Card::MegaDragonite,
        Card::Dragapult,
        Card::GoodraVstar,
        Card::DialgaGX,
    }};

    [[nodiscard]] static int count(const std::vector<Card>& zone) {
      return static_cast<int>(std::count_if(zone.begin(), zone.end(), is_payload));
    }

    template <typename Predicate>
    [[nodiscard]] static std::optional<Card> first(const Predicate& present) {
      for (const Card card : kOrder) {
        if (present(card)) return card;
      }
      return std::nullopt;
    }
  };

  [[nodiscard]] int payload_cards_in_hand_count() const {
    return PayloadPreferencePolicy::count(state_.hand);
  }

  [[nodiscard]] std::optional<Card> first_payload_card_in_hand() const {
    // Keep the simulator's established payload preference order while centralizing
    // repeated DCI/JIT hand scans:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    return PayloadPreferencePolicy::first(
        [this](const Card card) { return hand_count(card) > 0; });
  }

  [[nodiscard]] std::optional<Card> first_searchable_payload_in_known_deck() const {
    // Once K1 has established the remaining deck, a deck-search Supporter must be
    // gated by a payload that is physically searchable in that deck rather than by
    // a payload already held in hand.
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed Burnet payload-zone bug: https://github.com/FlareZ123/pokemon-sims/issues/3302
    return PayloadPreferencePolicy::first([this](const Card card) {
      return deck_count_after_search_started(card) > 0;
    });
  }

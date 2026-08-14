  struct PayloadPreferencePolicy {
    inline static constexpr std::array<Card, 5> kOrder{{
        Card::Appletun,
        Card::MegaDragonite,
        Card::Dragapult,
        Card::GoodraVstar,
        Card::DialgaGX,
    }};

    [[nodiscard]] static bool contains(const std::vector<Card>& zone) {
      return std::any_of(zone.begin(), zone.end(), is_payload);
    }

    [[nodiscard]] static int count(const std::vector<Card>& zone) {
      return static_cast<int>(std::count_if(zone.begin(), zone.end(), is_payload));
    }

    [[nodiscard]] static std::optional<Card> first_in_zone(
        const std::vector<Card>& zone) {
      const auto found = std::find_if(zone.begin(), zone.end(), is_payload);
      return found == zone.end() ? std::nullopt : std::optional<Card>{*found};
    }

    template <typename Predicate>
    [[nodiscard]] static std::optional<Card> first(const Predicate& present) {
      for (const Card card : kOrder) {
        if (present(card)) return card;
      }
      return std::nullopt;
    }
  };

  [[nodiscard]] static bool payload_zone_contains(const std::vector<Card>& zone) {
    // Centralize the simulator's Dragon-payload membership test without moving
    // DCI/JIT route admission out of Engine strategy:
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    return PayloadPreferencePolicy::contains(zone);
  }

  [[nodiscard]] static int payload_card_count(const std::vector<Card>& zone) {
    return PayloadPreferencePolicy::count(zone);
  }

  [[nodiscard]] static std::optional<Card> first_payload_card_in_zone(
      const std::vector<Card>& zone) {
    // Preserve physical zone order when a route needs the same first Dragon payload
    // that its historical std::find_if scan would have selected:
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    return PayloadPreferencePolicy::first_in_zone(zone);
  }

  [[nodiscard]] int payload_cards_in_hand_count() const {
    return payload_card_count(state_.hand);
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

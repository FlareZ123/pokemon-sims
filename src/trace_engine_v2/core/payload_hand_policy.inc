  struct PayloadZonePolicy {
    [[nodiscard]] static auto first_iterator(const std::vector<Card>& zone) {
      // Keep the physical-zone membership seam on the same first-match traversal
      // used by first(); callers that need strategic ordering use PayloadPreferencePolicy.
      // https://eel.is/c++draft/alg.find
      // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      return std::find_if(zone.begin(), zone.end(), is_payload);
    }

    [[nodiscard]] static bool contains(const std::vector<Card>& zone) {
      return first_iterator(zone) != zone.end();
    }

    [[nodiscard]] static int count(const std::vector<Card>& zone) {
      return static_cast<int>(std::count_if(zone.begin(), zone.end(), is_payload));
    }

    [[nodiscard]] static std::optional<Card> first(
        const std::vector<Card>& zone) {
      const auto found = first_iterator(zone);
      return found == zone.end() ? std::nullopt : std::optional<Card>{*found};
    }
  };

  struct PayloadPreferencePolicy {
    inline static constexpr std::array<Card, 5> kOrder{{
        Card::Appletun,
        Card::MegaDragonite,
        Card::Dragapult,
        Card::GoodraVstar,
        Card::DialgaGX,
    }};

    template <typename Predicate>
    [[nodiscard]] static std::optional<Card> first_preferred(
        const Predicate& present) {
      for (const Card card : kOrder) {
        if (present(card)) return card;
      }
      return std::nullopt;
    }

    [[nodiscard]] static bool contains_card(const std::vector<Card>& zone,
                                            const Card card) {
      // Centralize exact-card membership for strategic preference scans so callers
      // share one C++20 find seam while preserving the existing priority order.
      // https://eel.is/c++draft/alg.find
      return std::find(zone.begin(), zone.end(), card) != zone.end();
    }

    [[nodiscard]] static std::optional<Card> first_preferred_in_zone(
        const std::vector<Card>& zone) {
      // Preserve strategic payload priority while keeping direct zone membership
      // traversal inside the payload policy rather than at Engine callers.
      // https://eel.is/c++draft/alg.find
      // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      return first_preferred(
          [&zone](const Card card) { return contains_card(zone, card); });
    }

    template <typename CountFunction>
    [[nodiscard]] static std::optional<Card> first_preferred_with_positive_count(
        const CountFunction& count) {
      // Keep one preference-order implementation for zones exposed through count
      // accessors. DCI/JIT admission remains at the route caller:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      return first_preferred(
          [&count](const Card card) { return count(card) > 0; });
    }
  };

  [[nodiscard]] static bool payload_zone_contains(const std::vector<Card>& zone) {
    // Centralize the simulator's Dragon-payload membership test without moving
    // DCI/JIT route admission out of Engine strategy:
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    return PayloadZonePolicy::contains(zone);
  }

  [[nodiscard]] static int payload_card_count(const std::vector<Card>& zone) {
    return PayloadZonePolicy::count(zone);
  }

  [[nodiscard]] static std::optional<Card> first_payload_card_in_zone(
      const std::vector<Card>& zone) {
    // Preserve physical zone order when a route needs the same first Dragon payload
    // that its historical std::find_if scan would have selected:
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    return PayloadZonePolicy::first(zone);
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
    return PayloadPreferencePolicy::first_preferred_in_zone(state_.hand);
  }

  [[nodiscard]] std::optional<Card> first_searchable_payload_in_known_deck() const {
    // Once K1 has established the remaining deck, a deck-search Supporter must be
    // gated by a payload that is physically searchable in that deck rather than by
    // a payload already held in hand.
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed Burnet payload-zone bug: https://github.com/FlareZ123/pokemon-sims/issues/3302
    return PayloadPreferencePolicy::first_preferred_with_positive_count(
        [this](const Card card) { return deck_count_after_search_started(card); });
  }
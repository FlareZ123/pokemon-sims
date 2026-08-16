  class PayloadZonePolicy final {
   public:
    template <typename Predicate>
    [[nodiscard]] static auto first_iterator_matching(
        const std::vector<Card>& zone, Predicate predicate) {
      // Keep physical-zone traversal behind one primitive so payload and concrete
      // card membership preserve identical first-match ordering.
      // C++20 find_if: https://eel.is/c++draft/alg.find
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      return std::find_if(zone.begin(), zone.end(), predicate);
    }

    template <typename Predicate>
    [[nodiscard]] static bool contains_matching(
        const std::vector<Card>& zone, Predicate predicate) {
      return first_iterator_matching(zone, predicate) != zone.end();
    }

    template <typename Predicate>
    [[nodiscard]] static int count_matching(
        const std::vector<Card>& zone, Predicate predicate) {
      // Keep payload cardinality on the same predicate-oriented zone seam as
      // membership so later cleanup does not recreate route-local count scans.
      // C++20 count_if: https://eel.is/c++draft/alg.count
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      return static_cast<int>(std::count_if(zone.begin(), zone.end(), predicate));
    }

    [[nodiscard]] static bool contains(const std::vector<Card>& zone) {
      return contains_matching(zone, is_payload);
    }

    [[nodiscard]] static bool contains_card(
        const std::vector<Card>& zone, const Card card) {
      // Keep concrete-card membership in the same physical-zone abstraction as
      // payload membership so strategic preference callers do not grow ad hoc scans.
      // https://eel.is/c++draft/alg.find
      // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      return contains_matching(
          zone, [card](const Card candidate) { return candidate == card; });
    }

    [[nodiscard]] static int count(const std::vector<Card>& zone) {
      return count_matching(zone, is_payload);
    }

    [[nodiscard]] static std::optional<Card> first(
        const std::vector<Card>& zone) {
      // Preserve physical zone order through the canonical first-match primitive:
      // https://eel.is/c++draft/alg.find
      // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      const auto found = first_iterator_matching(zone, is_payload);
      return found == zone.end() ? std::nullopt : std::optional<Card>{*found};
    }
  };

  class PayloadPreferencePolicy final {
   public:
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

    [[nodiscard]] static std::optional<Card> first_preferred_in_zone(
        const std::vector<Card>& zone) {
      // Preserve strategic payload priority while keeping direct zone membership
      // traversal inside the payload policy rather than at Engine callers.
      // https://eel.is/c++draft/alg.find
      // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
      return first_preferred([&zone](const Card card) {
        return PayloadZonePolicy::contains_card(zone, card);
      });
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

  [[nodiscard]] static std::optional<Card> first_payload_card_in_zone(
      const std::vector<Card>& zone) {
    // Preserve physical zone order when a route needs the same first Dragon payload
    // that its historical std::find_if scan would have selected:
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    return PayloadZonePolicy::first(zone);
  }

  [[nodiscard]] int payload_cards_in_hand_count() const {
    // Keep count semantics at the canonical physical-zone owner:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#payload-policy-cleanup
    return PayloadZonePolicy::count(state_.hand);
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

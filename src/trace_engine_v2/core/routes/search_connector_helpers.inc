  bool need_regi() const { return !has_any_regi(); }

  // The VSTAR card axis is complete once the card is in hand; evolve_best_regi
  // resolves the later game-state transition when its timing condition is legal.
  bool need_vstar() const {
    return has_regi_v() && !has_vstar() &&
           hand_count(Card::RegidragoVstar) == 0;
  }

  class SearchConnectorFallbackPolicy final {
   public:
    [[nodiscard]] static constexpr std::array<Card, 11>
    mysterious_treasure_targets() {
      // This is the K1 post-search fallback legality/order, distinct from the
      // route's strategic MysteriousTreasureTargetPolicy priority.
      // Mysterious Treasure searches Psychic or Dragon Pokemon: https://api.pokemontcg.io/v2/cards/sm6-113
      // Appletun and Dipplin are legal Dragon targets: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/sv6-127
      // Crobat V is Darkness and is intentionally absent: https://api.pokemontcg.io/v2/cards/swsh3-104
      return {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
              Card::Oricorio, Card::TapuLeleGX, Card::MegaDragonite,
              Card::Dragapult, Card::GoodraVstar, Card::DialgaGX,
              Card::Appletun, Card::Dipplin};
    }

    [[nodiscard]] static constexpr std::array<Card, 8> quick_ball_targets() {
      // Quick Ball searches Basic Pokemon: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Pineco remains a legal modeled Basic fallback: https://api.pokemontcg.io/v2/cards/sv4pt5-1
      return {Card::RegidragoV, Card::LatiasEx, Card::Oricorio,
              Card::TapuLeleGX, Card::CrobatV, Card::MawileGX,
              Card::DialgaGX, Card::Pineco};
    }

    [[nodiscard]] static constexpr std::array<Card, 15> ultra_ball_targets() {
      // Ultra Ball searches any Pokemon: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // Appletun and the Pineco/Forretress package remain modeled legal targets: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
      return {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
              Card::Oricorio, Card::TapuLeleGX, Card::CrobatV,
              Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
              Card::DialgaGX, Card::MawileGX, Card::Dipplin,
              Card::Appletun, Card::Pineco, Card::ForretressEx};
    }

    template <std::size_t N, typename DeckCount>
    [[nodiscard]] static Card first_available(
        const std::array<Card, N>& targets, const Card fallback,
        const DeckCount& deck_count) {
      // K1 fallback callers have already started a legal deck search, so they can
      // traverse the resolved deck zone in printed-effect-specific order:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
      for (const Card target : targets) {
        if (deck_count(target) > 0) return target;
      }
      return fallback;
    }
  };

  bool search_outlet_present_after_current_search(
      const Card outlet, const Card excluded) const {
    return hand_count(outlet) > (outlet == excluded ? 1 : 0);
  }

  bool hand_discard_outlet_survives_current_search(
      const Card outlet, const Card excluded) const {
    const auto cost = choose_discard(false, true);
    if (!cost) return false;
    const int played_copy = outlet == excluded ? 1 : 0;
    const int cost_copy = *cost == outlet ? 1 : 0;
    return hand_count(outlet) > played_copy + cost_copy;
  }

  bool ultra_ball_outlet_payable_after_current_search(
      const Card excluded) const {
    return search_outlet_present_after_current_search(Card::UltraBall, excluded) &&
           state_.hand.size() >= 4U;
  }

  bool has_other_payload_outlet(const Card excluded) const {
    // A search connector can only fetch a future payload when a payload may still
    // be in deck. K1 derives this from legal deck or Prize inspection.
    if (!payload_might_be_in_deck()) return false;
    // The fetched payload is in hand, so the continuation must discard from hand.
    // Mysterious Treasure, Quick Ball, Ultra Ball, and Earthen Vessel have hand-discard
    // costs: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/sv4-163
    // Serena can discard from hand only while the turn's Supporter play remains available:
    // https://api.pokemontcg.io/v2/cards/swsh12-164 https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Evolution Incense has no discard effect, while Brilliant Blender and Professor Burnet
    // discard cards selected from deck, so none can dispose of this fetched hand payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // The current search Item and its mandatory discard resolve before the fetched
    // payload reaches hand. A claimed continuation must survive both operations;
    // Mysterious Treasure and Quick Ball both require that upfront discard:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179
    // The current Treasure or Quick Ball line spends its search card and one discard,
    // then adds the fetched payload. Ultra Ball still needs two other cards when played:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Its post-search hand therefore needs Ultra Ball, the fetched payload, and one
    // further card, which requires at least four cards in the pre-search hand.
    return hand_discard_outlet_survives_current_search(Card::MysteriousTreasure,
                                                       excluded) ||
           hand_discard_outlet_survives_current_search(Card::QuickBall, excluded) ||
           ultra_ball_outlet_payable_after_current_search(excluded) ||
           hand_discard_outlet_survives_current_search(Card::EarthenVessel,
                                                       excluded) ||
           (supporter_allowed() &&
            search_outlet_present_after_current_search(Card::Serena, excluded));
  }

  Card fallback_mysterious_target_after_search_started() const {
    // Legal K1 fallback coverage remains separate from strategic route priority.
    // Confirmed fallback coverage: https://github.com/FlareZ123/pokemon-sims/issues/1253 https://github.com/FlareZ123/pokemon-sims/issues/1949
    return SearchConnectorFallbackPolicy::first_available(
        SearchConnectorFallbackPolicy::mysterious_treasure_targets(),
        Card::RegidragoV,
        [this](const Card card) { return deck_count_after_search_started(card); });
  }

  Card fallback_quick_ball_target_after_search_started() const {
    // Confirmed Pineco fallback coverage: https://github.com/FlareZ123/pokemon-sims/issues/1958
    return SearchConnectorFallbackPolicy::first_available(
        SearchConnectorFallbackPolicy::quick_ball_targets(), Card::RegidragoV,
        [this](const Card card) { return deck_count_after_search_started(card); });
  }

  Card fallback_ultra_ball_target_after_search_started() const {
    // Confirmed fallback coverage: https://github.com/FlareZ123/pokemon-sims/issues/1254 https://github.com/FlareZ123/pokemon-sims/issues/1952
    return SearchConnectorFallbackPolicy::first_available(
        SearchConnectorFallbackPolicy::ultra_ball_targets(), Card::RegidragoV,
        [this](const Card card) { return deck_count_after_search_started(card); });
  }

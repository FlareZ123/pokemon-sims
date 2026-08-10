  int payload_cards_in_hand_count() const {
    return static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_payload));
  }

  std::optional<Card> first_payload_card_in_hand() const {
    // Keep the simulator's established payload preference order while centralizing
    // repeated DCI/JIT hand scans:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    static constexpr Card kPayloadPreference[] = {
        Card::Appletun,
        Card::MegaDragonite,
        Card::Dragapult,
        Card::GoodraVstar,
        Card::DialgaGX,
    };
    for (const Card card : kPayloadPreference) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard_issue1436_original(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    if (ordinary || scenario_.locks != LockMode::FullSupporter) return ordinary;

    // Full Supporter lock disables every Supporter effect for the complete
    // modeled horizon. A held Supporter therefore has zero Supporter AMR and may
    // pay an Item's printed discard cost after every ordinary lower-DCI option
    // has been exhausted. The card being played remains excluded from its own
    // cost, and every non-Supporter singleton keeps its existing protection:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1436
    for (const Card card : state_.hand) {
      if (!is_supporter(card)) continue;
      const int excluded = excluded_from_cost && *excluded_from_cost == card ? 1 : 0;
      if (hand_count(card) > excluded) return card;
    }
    return std::nullopt;
  }

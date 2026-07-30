  std::optional<Card> issue_1876_treasure_payload_cost() const {
    const bool complete_public_route =
        scenario_.dci == DciProfile::MatchupFlexJit && deck_seen_ &&
        !item_locked() && supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::Crispin) > 0 &&
        crispin_can_advance_energy_axis() &&
        std::any_of(state_.deck.begin(), state_.deck.end(),
                    is_dragon_or_psychic);
    if (!complete_public_route) return std::nullopt;

    // Mysterious Treasure may discard any card. In this public K1 GF state,
    // held Crispin attaches the missing Grass, so a visible Dragon is DCI 1
    // because this exact cost creates the current-turn Apex Dragon payload.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dialga-GX and Dragapult ex: https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Supporter, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1876
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> choose_discard_issue1876(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    if (excluded_from_cost == Card::MysteriousTreasure) {
      if (const auto payload = issue_1876_treasure_payload_cost()) {
        return payload;
      }
    }
    return choose_discard_issue1740(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
  }

#undef choose_discard
#define choose_discard choose_discard_issue1876

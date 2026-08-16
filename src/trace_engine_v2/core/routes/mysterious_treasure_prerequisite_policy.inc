  std::optional<Card> issue_1221_burnet_treasure_vstar_cost() const {
    // After Professor Burnet establishes the strict current-turn payload, one
    // setup-dead Supporter may pay Mysterious Treasure only when the searched
    // Regidrago VSTAR immediately completes an evolution-legal Active GGF route:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, discard, search, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Strict-JIT and DCI specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1221
    const bool burnet_resolved_this_turn = state_.supporter_used &&
        count_of(state_.discard, Card::ProfessorBurnet) > 0 &&
        payload_zone_contains(state_.discarded_this_turn);
    const bool active_route_complete = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        pays_apex_energy_cost(*state_.active) && // https://github.com/FlareZ123/pokemon-sims/issues/2368
        !item_locked() && hand_count(Card::MysteriousTreasure) > 0 &&
        might_be_unseen(Card::RegidragoVstar);
    if (!burnet_resolved_this_turn || !active_route_complete) return std::nullopt;

    // These Supporters have no remaining setup role after Burnet has consumed the
    // Supporter play and every non-VSTAR axis is complete. Movement, recovery,
    // Energy, ACE SPEC, Pokémon, and Tool resources remain protected.
    for (const Card card : {Card::Channeler, Card::ErikasInvitation}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1221_burnet_then_treasure_route_available() const {
    if (!supporter_allowed() || item_locked() ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !payload_might_be_in_deck() ||
        !might_be_unseen(Card::RegidragoVstar) || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        // DDE supplies two all-type Energy to a Dragon Pokémon; Apex Dragon costs [G][G][R]:
        // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ https://api.pokemontcg.io/v2/cards/swsh12-136
        !pays_apex_energy_cost(*state_.active)) {
      return false;
    }
    return hand_count(Card::Channeler) > 0 ||
           hand_count(Card::ErikasInvitation) > 0;
  }

  bool mysterious_treasure_search_has_target() const {
    // Mysterious Treasure searches only Psychic or Dragon Pokémon:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // K0 keeps unaccounted fixed-list targets plausible, while K1 uses the inspected deck exactly:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    if (prizes_known()) {
      return std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    }
    return std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
      return is_dragon_or_psychic(entry.first) && might_be_unseen(entry.first);
    });
  }

  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt) const {
    const bool setup_before_steven_route =
        excluded_from_cost && *excluded_from_cost == Card::MysteriousTreasure &&
        state_.turn == 1 && !scenario_.going_first && !item_locked() &&
        supporter_allowed() && need_regi() && bench_space() > 0 &&
        can_free_retreat_with_latias() && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::Fire) == 1 && hand_count(Card::Grass) > 0 &&
        might_be_unseen(Card::RegidragoV) && might_be_unseen(Card::RegidragoVstar) &&
        might_be_unseen(Card::Crispin) && might_be_unseen(Card::BrilliantBlender) &&
        might_be_unseen(Card::Grass) && might_be_unseen(Card::Fire);
    if (setup_before_steven_route) {
      // Spending the public Fire Energy lets Mysterious Treasure establish
      // Regidrago V before Steven's Resolve ends turn one. Grass remains for the
      // manual attachment, Crispin restores GGF on turn two, and Latias ex supplies
      // the proven promotion route:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/775
      return Card::Fire;
    }
    return choose_discard_original(permit_payload, flex_fodder,
                                   allow_heavy_ball, excluded_from_cost);
  }

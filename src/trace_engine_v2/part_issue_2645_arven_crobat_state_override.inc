  bool issue_1605_arven_crobat_route_available() const {
    const bool one_discard_item_plausible =
        might_be_unseen(Card::QuickBall) ||
        might_be_unseen(Card::MysteriousTreasure);

    // The Arven -> one-discard search -> Regidrago V -> Forest Seal Stone ->
    // Crobat V continuation is governed by observable legality and resources.
    // supporter_allowed() already enforces the first player's T1 Supporter ban,
    // Supporter lock, prior Supporter use, and turn-end state, so turn/order
    // identity must not suppress the same legal continuation on T2 or later.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core first-turn, Supporter, Item, Bench, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // State-based earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed overfit bug: https://github.com/FlareZ123/pokemon-sims/issues/2645
    return scenario_.dci == DciProfile::StrictJit &&
           supporter_allowed() && !item_locked() && need_regi() &&
           bench_space() >= 2 && hand_count(Card::Arven) > 0 &&
           hand_count(Card::CrobatV) > 0 &&
           hand_count(Card::ForestSealStone) > 0 &&
           !state_.vstar_power_used && !state_.dark_asset_used &&
           ability_available_for_pokemon(Card::CrobatV) &&
           state_.hand.size() <= 9U &&
           might_be_unseen(Card::RegidragoV) &&
           one_discard_item_plausible &&
           issue_1605_redundant_payload_cost().has_value();
  }

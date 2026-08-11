  bool issue_1605_one_discard_search_item_plausible() const {
    // The #1605 continuation needs a one-discard Basic/Dragon search channel.
    // Keep target visibility on the existing K0/K1-aware might_be_unseen() path:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return might_be_unseen(Card::QuickBall) ||
           might_be_unseen(Card::MysteriousTreasure);
  }

  bool issue_1605_crobat_resources_available() const {
    // Keep the connector's Bench, hand-compression, Tool, Ability, and once-per-game
    // resources in one public-state preflight:
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Core Bench, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return bench_space() >= 2 && hand_count(Card::Arven) > 0 &&
           hand_count(Card::CrobatV) > 0 &&
           hand_count(Card::ForestSealStone) > 0 &&
           !state_.vstar_power_used && !state_.dark_asset_used &&
           ability_available_for_pokemon(Card::CrobatV) &&
           state_.hand.size() <= 9U;
  }

  bool issue_1605_regidrago_search_route_available() const {
    // Group the Item legality, Regidrago target, and dynamic-DCI payment gates that
    // make the one-discard search connector executable from the public state:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core Item and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Completed route fix: https://github.com/FlareZ123/pokemon-sims/issues/1605
    return !item_locked() && need_regi() && might_be_unseen(Card::RegidragoV) &&
           issue_1605_one_discard_search_item_plausible() &&
           issue_1605_redundant_payload_cost().has_value();
  }

  bool issue_1605_arven_crobat_route_available() const {
    // The Arven -> one-discard search -> Regidrago V -> Forest Seal Stone ->
    // Crobat V continuation is governed by observable legality and resources.
    // supporter_allowed() already enforces the first player's T1 Supporter ban,
    // Supporter lock, prior Supporter use, and turn-end state, so turn/order
    // identity must not suppress the same legal continuation on T2 or later.
    // StrictJit and MatchupFlexJit share the same ready-turn payload contract;
    // NoDiscardControl remains outside this redundant-payload fallback.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core first-turn, Supporter, Item, Bench, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // State-based earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Prior state-based overfit fix: https://github.com/FlareZ123/pokemon-sims/issues/2645
    // Confirmed shared-JIT follow-up: https://github.com/FlareZ123/pokemon-sims/issues/3132
    return strict_payload_timing() && supporter_allowed() &&
           issue_1605_regidrago_search_route_available() &&
           issue_1605_crobat_resources_available();
  }
  bool held_ultra_ball_can_find_regidrago_now() const {
    if (item_locked() || hand_count(Card::UltraBall) == 0 ||
        !might_be_unseen(Card::RegidragoV)) {
      return false;
    }

    std::mt19937_64 shadow_rng{779};
    Engine shadow(scenario_, recipe_, shadow_rng);
    shadow.state_ = state_;
    shadow.deck_seen_ = deck_seen_;
    shadow.prizes_revealed_ = prizes_revealed_;
    if (!remove_one(shadow.state_.hand, Card::UltraBall)) return false;

    const bool permit_payload = shadow.can_play_payload_this_turn();
    const auto first = shadow.choose_discard(permit_payload, true);
    if (!first || !remove_one(shadow.state_.hand, *first)) return false;
    return shadow.choose_discard(permit_payload && !is_payload(*first), true).has_value();
  }

  bool has_faster_current_turn_regidrago_v_route() const {
    if (item_locked() || !might_be_unseen(Card::RegidragoV)) return false;

    const bool permit_payload = can_play_payload_this_turn();
    const bool quick_ball_route = hand_count(Card::QuickBall) > 0 &&
        choose_discard(permit_payload, true, true, Card::QuickBall).has_value();
    const bool treasure_route = hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload, true, true, Card::MysteriousTreasure).has_value();
    const bool communication_route = hand_count(Card::PokemonCommunication) > 0;
    const bool arven_route = hand_count(Card::Arven) > 0 &&
        is_legal_cost_available(permit_payload, true) &&
        (might_be_unseen(Card::QuickBall) || might_be_unseen(Card::MysteriousTreasure));

    // Quick Ball and Mysterious Treasure each need another discard, Ultra Ball needs
    // two, and Pokémon Communication needs a Pokémon to return. Conservatively keep
    // Steven behind a held Communication until its exchange planner has resolved:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/sv1-166
    return quick_ball_route || treasure_route || held_ultra_ball_can_find_regidrago_now() ||
        communication_route || arven_route;
  }

  bool late_steven_is_only_basic_connector() const {
    if (state_.turn < 2 || !supporter_allowed() || !need_regi() ||
        bench_space() == 0 || hand_count(Card::RegidragoV) > 0 ||
        !might_be_unseen(Card::RegidragoV)) {
      return false;
    }

    const bool unused_fss_route = !state_.vstar_power_used &&
        ((state_.active && state_.active->tool == Tool::ForestSealStone) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
           return pokemon.tool == Tool::ForestSealStone;
         }));

    // Steven's Resolve is legal on every Supporter-legal turn and may search any
    // three cards. When Regidrago V is absent, no payable same-turn Basic search or
    // unused Star Alchemy route exists, taking Regidrago V is strictly better than
    // taking no advancing action even though Steven ends the current turn:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/779
    return !unused_fss_route && !has_faster_current_turn_regidrago_v_route();
  }

  bool should_play_steven() const {
    return should_play_steven_original() || late_steven_is_only_basic_connector();
  }

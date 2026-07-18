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

    Engine communication_shadow = *this;
    const bool communication_route =
        communication_shadow.pokemon_communication_has_direct_target_route(
            Card::RegidragoV);

    const bool arven_search_item_route = hand_count(Card::Arven) > 0 &&
        is_legal_cost_available(permit_payload, true) &&
        (might_be_unseen(Card::QuickBall) || might_be_unseen(Card::MysteriousTreasure));
    Engine arven_communication_shadow = *this;
    const auto arven_communication_plan =
        arven_communication_shadow.pokemon_communication_plan_after_playing_supporter(
            Card::Arven, false);
    const bool arven_communication_route = hand_count(Card::Arven) > 0 &&
        might_be_unseen(Card::PokemonCommunication) &&
        arven_communication_plan &&
        arven_communication_plan->target == Card::RegidragoV;

    // Every faster Basic route is cost-aware. Quick Ball and Mysterious Treasure
    // each need another discard, Ultra Ball needs two, and Pokémon Communication
    // must have a safe hand Pokémon to return for an available Regidrago V target:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_pokemon_communication.inc#L42-L61
    return quick_ball_route || treasure_route || held_ultra_ball_can_find_regidrago_now() ||
        communication_route || arven_search_item_route || arven_communication_route;
  }

  bool late_steven_has_known_t3_compression_route() const {
    if (state_.turn != 2 || scenario_.locks != LockMode::None || !supporter_allowed() ||
        hand_count(Card::StevensResolve) == 0 || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::QuickBall) == 0 || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx) ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::MegaDragonite) == 0) {
      return false;
    }

    bool before_attachment = false;
    bool after_attachment = false;
    bool after_evolution = false;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.entered_turn >= state_.turn) continue;
      if (pokemon.card == Card::RegidragoV && pokemon.grass == 1 && pokemon.fire == 0 &&
          !state_.manual_energy_used && hand_count(Card::RegidragoVstar) > 0 &&
          hand_count(Card::Fire) > 0 && hand_count(Card::Grass) > 0) {
        before_attachment = true;
      }
      if (pokemon.card == Card::RegidragoV && pokemon.grass == 1 && pokemon.fire == 1 &&
          state_.manual_energy_used && hand_count(Card::RegidragoVstar) > 0 &&
          hand_count(Card::Grass) > 0) {
        after_attachment = true;
      }
      if (pokemon.card == Card::RegidragoVstar && pokemon.grass == 1 && pokemon.fire == 1 &&
          state_.manual_energy_used && hand_count(Card::Grass) > 0) {
        after_evolution = true;
      }
    }

    // Steven's Resolve can bank Latias ex, Mysterious Treasure, and Mega Dragonite ex
    // after K1 proves all three remain in deck. Evolving the prior-turn Regidrago V
    // and attaching Fire before Steven preserves the held Grass attachment for T3;
    // Treasure then discards the held Dragon payload, Latias grants free Retreat, and
    // the route reaches Active GGF Regidrago VSTAR one turn earlier than Tate draw mode:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    return before_attachment || after_attachment || after_evolution;
  }

  bool play_known_steven_t3_payload_outlet() {
    if (state_.turn != 3 || item_locked() || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::MegaDragonite) == 0 || hand_count(Card::Grass) == 0 ||
        state_.manual_energy_used || !need_payload() || !in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }
    const bool prepared_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar && pokemon.grass == 1 &&
              pokemon.fire == 1 && pokemon.entered_turn < state_.turn;
        });
    const bool legal_search_target = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    if (!prepared_vstar || !legal_search_target) return false;

    // This exact K1 continuation uses Mysterious Treasure before Tate & Liza can
    // shuffle away the held outlet and payload. The printed Item discards one card,
    // then searches a Psychic or Dragon Pokémon; Mega Dragonite ex is the deliberate
    // Apex Dragon payload and the held Grass plus Latias ex prove the T3 finish:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(Card::MegaDragonite, "Known Steven Treasure payload cost", "R-MT-01");
    record_deck_search_knowledge("Mysterious Treasure");
    const Card target = fallback_mysterious_target_after_search_started();
    const bool found = move_deck_to_hand(target);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-COMPRESS-01",
          found ? "Preserved the known Steven T3 payload route and searched " +
                      std::string(name(target)) + "."
                : "Preserved the known Steven T3 payload route; no legal target remained after search.");
    return true;
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

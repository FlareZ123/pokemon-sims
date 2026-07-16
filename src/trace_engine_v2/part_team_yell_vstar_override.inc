  bool has_payable_team_yell_vstar_search() {
    if (item_locked()) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    if (!remove_one(state_.hand, Card::TeamYellsCheer)) return false;

    // Evolution Incense searches an Evolution Pokémon without a discard cost:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    if (hand_count(Card::EvolutionIncense) > 0) {
      restore_hand();
      return true;
    }

    // Team Yell's Cheer restores Regidrago VSTAR to the deck before the held Item
    // resolves. Pokémon Communication is live only when the post-Supporter hand has
    // a safe Pokémon to exchange for that exact restored target:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    restore_hand();
    if (pokemon_communication_can_find_after_recovery(
            Card::RegidragoVstar, Card::TeamYellsCheer,
            {Card::RegidragoVstar})) {
      return true;
    }
    if (!remove_one(state_.hand, Card::TeamYellsCheer)) return false;

    // Mysterious Treasure must discard another hand card before searching a Dragon
    // Pokémon. Test the cost after removing the Supporter that will already be played:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    const bool permit_payload = can_play_payload_this_turn();
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload, true, true, Card::MysteriousTreasure).has_value()) {
      restore_hand();
      return true;
    }

    // Ultra Ball must still have two other policy-legal cards after Team Yell's Cheer
    // leaves the hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (hand_count(Card::UltraBall) > 0) {
      remove_one(state_.hand, Card::UltraBall);
      const auto first = choose_discard(permit_payload, true);
      if (first && remove_one(state_.hand, *first)) {
        const auto second = choose_discard(permit_payload && !is_payload(*first), true);
        if (second) {
          restore_hand();
          return true;
        }
      }
    }

    restore_hand();
    return false;
  }

  bool play_team_yell_vstar_recovery() {
    if (!supporter_allowed() || hand_count(Card::TeamYellsCheer) == 0 || !need_vstar() ||
        !has_regidrago_v_that_can_evolve_this_turn() ||
        count_of(state_.discard, Card::RegidragoVstar) == 0) {
      return false;
    }

    // Preserve a direct deck search when a Regidrago VSTAR may already remain in the
    // deck. This recovery is selected after legal knowledge proves the search target
    // absent and a discarded copy can restore a payable same-turn route.
    if (might_be_unseen(Card::RegidragoVstar) || !has_payable_team_yell_vstar_search()) return false;

    const auto evolution_incense_then_latias_search_completes_payload_axis = [this]() {
      if (item_locked() || hand_count(Card::EvolutionIncense) == 0) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      // The Supporter and Evolution Incense both leave hand before the later Basic or
      // Psychic search pays its cost. Test that exact remaining hand so one card cannot
      // satisfy both future actions:
      // https://api.pokemontcg.io/v2/cards/swsh9-149
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sm6-113
      if (!remove_one(state_.hand, Card::TeamYellsCheer) ||
          !remove_one(state_.hand, Card::EvolutionIncense)) {
        restore_hand();
        return false;
      }
      const bool payload_already_ready = payload_ready();
      const bool permit_payload = can_play_payload_this_turn();

      // Evolution Incense first searches the restored VSTAR. Pokémon Communication
      // may then exchange a remaining hand Pokémon for the separately restored
      // Latias ex. Because the exchange does not discard a Dragon, this continuation
      // is valid only when the strict-JIT payload axis is already complete:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sm9-152
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/701
      if (payload_already_ready && hand_count(Card::PokemonCommunication) > 0) {
        Engine simulated = *this;
        if (remove_one(simulated.state_.hand, Card::TeamYellsCheer) &&
            remove_one(simulated.state_.hand, Card::EvolutionIncense)) {
          simulated.state_.deck.push_back(Card::RegidragoVstar);
          simulated.state_.deck.push_back(Card::LatiasEx);
          if (simulated.move_deck_to_hand(Card::RegidragoVstar)) {
            const auto plan = simulated.pokemon_communication_plan(false, true, true);
            if (plan && plan->target == Card::LatiasEx) {
              restore_hand();
              return true;
            }
          }
        }
      }

      for (const Card item : {Card::QuickBall, Card::MysteriousTreasure}) {
        if (hand_count(item) == 0) continue;
        const auto cost = choose_discard(permit_payload, true, true, item);
        // Quick Ball and Mysterious Treasure pay their discard costs before searching.
        // A permitted Dragon cost therefore establishes the strict-JIT Apex Dragon
        // payload before the restored Latias ex is found and Benched:
        // https://api.pokemontcg.io/v2/cards/swsh1-179
        // https://api.pokemontcg.io/v2/cards/sm6-113
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        if (cost && (payload_already_ready || (permit_payload && is_payload(*cost)))) {
          restore_hand();
          return true;
        }
      }
      if (hand_count(Card::UltraBall) > 0) {
        remove_one(state_.hand, Card::UltraBall);
        const auto first = choose_discard(permit_payload, true);
        if (first && remove_one(state_.hand, *first)) {
          const auto second = choose_discard(permit_payload && !is_payload(*first), true);
          // Ultra Ball's two-card cost may establish the same strict-JIT payload axis
          // when either selected card is a permitted Dragon payload:
          // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
          // https://api.pokemontcg.io/v2/cards/swsh12-136
          if (second && (payload_already_ready || is_payload(*first) || is_payload(*second))) {
            restore_hand();
            return true;
          }
        }
      }
      restore_hand();
      return false;
    };

    // Team Yell's Cheer may return up to 3 eligible cards in one resolution. When a
    // powered prior-turn Benched Regidrago V will evolve behind a Basic Active, restore
    // Latias ex together with Regidrago VSTAR if Evolution Incense and a separate
    // Basic or Psychic search remain payable. That later search may itself discard the
    // held strict-JIT payload before finding Latias ex:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Retreat_Your_Active_Pokemon
    const bool powered_benched_regi_can_evolve = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn &&
                 pokemon.grass >= 2 && pokemon.fire >= 1;
        });
    // An Active Regidrago V makes Latias redundant only when it can legally evolve now
    // and already carries GGF for Apex Dragon. A Regidrago V that entered this turn or
    // lacks the attack cost cannot replace the ready Benched route:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool active_regi_can_be_ready_without_retreat = state_.active &&
        state_.active->card == Card::RegidragoV && state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire >= 1;
    const bool restore_latias_for_promotion = state_.active && is_basic(state_.active->card) &&
        !active_regi_can_be_ready_without_retreat && powered_benched_regi_can_evolve &&
        !state_.retreat_used && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && count_of(state_.discard, Card::LatiasEx) > 0 &&
        !might_be_unseen(Card::LatiasEx) &&
        evolution_incense_then_latias_search_completes_payload_axis();

    remove_one(state_.hand, Card::TeamYellsCheer);
    state_.discard.push_back(Card::TeamYellsCheer);
    state_.supporter_used = true;

    const auto restore_to_deck = [this](const Card card) {
      if (!remove_one(state_.discard, card)) {
        throw std::logic_error("Team Yell's Cheer recovery target disappeared");
      }
      const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                     state_.discarded_this_turn.end(), card);
      if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
      state_.deck.push_back(card);
    };

    // Team Yell's Cheer may shuffle up to 3 Pokémon and/or Supporter cards other than
    // itself from discard into the deck. The restored VSTAR may evolve any legal
    // prior-turn Regidrago V in play, including one on the Bench:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://www.pokemon.com/us/pokemon-tcg/rules
    restore_to_deck(Card::RegidragoVstar);
    if (restore_latias_for_promotion) restore_to_deck(Card::LatiasEx);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          restore_latias_for_promotion
              ? "Team Yell's Cheer restored Regidrago VSTAR and Latias ex for Evolution Incense, a separate Latias search, and promotion."
              : "Team Yell's Cheer shuffled Regidrago VSTAR from discard into the deck for a payable same-turn search.");
    return true;
  }

  bool has_payable_team_yell_latias_search() {
    if (item_locked()) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    if (!remove_one(state_.hand, Card::TeamYellsCheer)) return false;
    const bool permit_payload = can_play_payload_this_turn();
    const bool payload_already_ready = payload_ready();

    // Pokémon Communication can search the restored Latias ex when a safe exchange
    // Pokémon remains after Team Yell's Cheer leaves hand. It cannot establish the
    // strict-JIT payload axis, so a missing payload still requires the discard-search
    // routes below:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    restore_hand();
    if (payload_already_ready &&
        pokemon_communication_can_find_after_recovery(
            Card::LatiasEx, Card::TeamYellsCheer, {Card::LatiasEx})) {
      return true;
    }
    if (!remove_one(state_.hand, Card::TeamYellsCheer)) return false;

    // Quick Ball and Mysterious Treasure each require one other hand card as their
    // discard cost before searching Latias ex. When the payload axis is missing, the
    // selected cost must itself be the Dragon that makes Apex Dragon ready this turn:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    for (const Card item : {Card::QuickBall, Card::MysteriousTreasure}) {
      if (hand_count(item) == 0) continue;
      const auto cost = choose_discard(permit_payload, true, true, item);
      if (cost && (payload_already_ready || is_payload(*cost))) {
        restore_hand();
        return true;
      }
    }

    // Ultra Ball requires two other cards after Team Yell's Cheer leaves hand. If
    // strict-JIT payload is still missing, one of those two costs must be the Dragon:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (hand_count(Card::UltraBall) > 0) {
      remove_one(state_.hand, Card::UltraBall);
      const auto first = choose_discard(permit_payload, true);
      if (first && remove_one(state_.hand, *first)) {
        const auto second = choose_discard(permit_payload && !is_payload(*first), true);
        if (second && (payload_already_ready || is_payload(*first) || is_payload(*second))) {
          restore_hand();
          return true;
        }
      }
    }

    restore_hand();
    return false;
  }

  bool play_team_yell_latias_recovery() {
    const bool powered_benched_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 && pokemon.fire >= 1;
        });
    const bool active_can_evolve_from_hand = state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn && hand_count(Card::RegidragoVstar) > 0;
    if (!supporter_allowed() || hand_count(Card::TeamYellsCheer) == 0 || !need_active_vstar() ||
        !state_.active || !is_basic(state_.active->card) || active_can_evolve_from_hand ||
        !powered_benched_vstar || state_.retreat_used || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
        count_of(state_.discard, Card::LatiasEx) == 0) {
      return false;
    }

    // If Latias ex may already remain in the deck, preserve Team Yell's Cheer and use
    // the direct search. The recovery route is for the known discarded singleton only.
    // The payable-search helper also proves that a missing strict-JIT payload will be
    // the search Item's discard cost instead of requiring it before that action:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (might_be_unseen(Card::LatiasEx) || !has_payable_team_yell_latias_search()) return false;

    remove_one(state_.hand, Card::TeamYellsCheer);
    state_.discard.push_back(Card::TeamYellsCheer);
    state_.supporter_used = true;

    // Team Yell's Cheer may return Latias ex to the deck. A payable Basic or Psychic
    // search can then find it, and Skyliner gives the Basic Active no Retreat Cost:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!remove_one(state_.discard, Card::LatiasEx)) {
      throw std::logic_error("Team Yell's Cheer Latias target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(),
                                   Card::LatiasEx);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(Card::LatiasEx);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Team Yell's Cheer shuffled Latias ex from discard into the deck for a payable same-turn search and free-retreat promotion.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (play_team_yell_vstar_recovery()) return;
    if (play_team_yell_latias_recovery()) return;
    choose_supporter_original();
  }

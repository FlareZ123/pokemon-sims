  Card choose_supporter_after_search_started_issue3200_original() const {
    return choose_supporter_after_search_started();
  }

  bool issue_3200_matchup_flex_t3_blender_initial_route() const {
    if (scenario_.dci != DciProfile::MatchupFlexJit) return false; // Same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    if (state_.turn != 1 || !scenario_.going_first ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !deck_seen_ || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::BrilliantBlender) == 0 || // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
        hand_count(Card::Grass) == 0 || hand_count(Card::Fire) == 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::StevensResolve) == 0 || // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
        deck_count_after_search_started(Card::RegidragoVstar) == 0 || // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    return std::any_of(state_.bench.begin(), state_.bench.end(),
        [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn <= state_.turn &&
              pokemon.grass == 0 && pokemon.fire == 0 &&
              pokemon.double_dragon == 0;
        }); // Evolution/turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  }

  Card choose_supporter_after_search_started() {
    if (issue_3200_matchup_flex_t3_blender_initial_route()) {
      return Card::StevensResolve; // Wonder Tag -> Steven banking route: https://api.pokemontcg.io/v2/cards/sm2-60
    }
    return choose_supporter_after_search_started_issue3200_original();
  }

  bool issue_3200_matchup_flex_t3_blender_banked_route() const {
    if (scenario_.dci != DciProfile::MatchupFlexJit || // Same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        hand_count(Card::BrilliantBlender) == 0 || !need_payload() ||
        !payload_might_be_in_deck()) {
      return false;
    }
    if (state_.turn != 2 || !scenario_.going_first ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) || // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
        in_play(Card::LatiasEx) || hand_count(Card::StevensResolve) == 0 || // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
        deck_count_after_search_started(Card::RegidragoVstar) == 0 || // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      if (pokemon.double_dragon > 0 && !pays_apex_energy_cost(pokemon) &&
          completing_basic_energy_for(pokemon, [this](const Card card) {
            return hand_count(card) > 0 ||
                (card == Card::Grass &&
                 deck_count_after_search_started(Card::Grass) > 0);
          }).has_value()) {
        return true;
      }

      int projected_grass = pokemon.grass;
      int projected_fire = pokemon.fire;
      int grass_in_hand = hand_count(Card::Grass);
      int fire_in_hand = hand_count(Card::Fire);
      if (!state_.manual_energy_used) {
        if (projected_grass < 2 && grass_in_hand > 0) {
          ++projected_grass;
          --grass_in_hand;
        } else if (projected_fire < 1 && fire_in_hand > 0) {
          ++projected_fire;
          --fire_in_hand;
        } else {
          continue;
        }
      }

      const int remaining_grass = std::max(0, 2 - projected_grass);
      const int remaining_fire = std::max(0, 1 - projected_fire);
      if ((remaining_grass == 1 && remaining_fire == 0) ||
          (remaining_grass == 0 && remaining_fire == 1 && fire_in_hand > 0)) {
        return true; // Apex Dragon GGF and normal attachment timing: https://api.pokemontcg.io/v2/cards/swsh12-136
      }
    }
    return false;
  }

  bool should_play_steven_issue3200() const {
    return should_play_steven_issue1030_original() ||
        issue_3200_matchup_flex_t3_blender_banked_route();
  }

  bool play_banked_steven_for_known_t3_blender_route() {
    if (!issue_3200_matchup_flex_t3_blender_banked_route()) {
      return play_banked_steven_for_known_t3_blender_route_issue3200_original();
    }

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> found;
    for (const Card target : {Card::RegidragoVstar, Card::Grass, Card::LatiasEx}) {
      if (!move_deck_to_hand(target)) {
        throw std::logic_error("Known MatchupFlexJit banked-Steven target disappeared");
      }
      found.push_back(target);
    }
    shuffle(state_.deck);
    state_.turn_ended = true; // Steven searches up to 3 cards, then the turn ends: https://api.pokemontcg.io/v2/cards/sm7-145

    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER",
          "Searched up to 3 cards: " + join_cards(found) + ". Turn ended.");
    return true;
  }

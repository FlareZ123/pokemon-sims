  bool legacy_star_delayed_vessel_route() const {
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const std::array<Card, 2> delayed_route_basics{Card::Grass, Card::Fire};
    const bool completing_basic_in_deck = state_.active && std::any_of(
        delayed_route_basics.begin(), delayed_route_basics.end(),
        [this](const Card energy) {
          if (count_of(state_.deck, energy) == 0) return false;
          Pokemon projected = *state_.active;
          return attach_energy_card(projected, energy) && pays_apex_energy_cost(projected);
        });
    // #1844 owns the canonical GF -> Grass delayed Vessel route. #2423 adds
    // the distinct DDE-only one-attachment state, where either Basic may finish.
    // Keep canonical GG with its existing missing-Fire route family.
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Original delayed Vessel boundary: https://github.com/FlareZ123/pokemon-sims/issues/1844
    // DDE migration contract: https://github.com/FlareZ123/pokemon-sims/issues/2423
    const bool canonical_gf_axis = state_.active &&
        state_.active->double_dragon == 0 && state_.active->grass == 1 &&
        state_.active->fire >= 1 && count_of(state_.deck, Card::Grass) > 0;
    const bool dde_only_axis = state_.active &&
        state_.active->double_dragon > 0 && state_.active->grass == 0 &&
        state_.active->fire == 0 && completing_basic_in_deck;
    const bool exact_reported_axis = state_.turn == 2 && active_is_vstar() &&
        (canonical_gf_axis || dde_only_axis);

    // Legacy Star may return Earthen Vessel now for a fully public next-turn
    // finish. The current turn's manual attachment is already spent, while the
    // next turn gets a fresh attachment. A held Dragon pays Vessel and enters
    // discard during that next strict-JIT ready turn:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Item, discard, search, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && deck_seen_ &&
        state_.turn + 1 <= scenario_.max_turn && state_.manual_energy_used &&
        exact_reported_axis && held_payload &&
        (count_of(state_.discard, Card::EarthenVessel) > 0 ||
         hand_count(Card::EarthenVessel) > 0);
  }

  bool use_legacy_star() {
    if (state_.vstar_power_used || !has_vstar() || !ability_available_for_pokemon(Card::RegidragoVstar)) return false;
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Use only when no direct payload outlet or direct connector remains and the
    // discard-7 can create a JIT payload line.
    const bool permit_payload_cost = can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const auto search_item_has_legal_target = [this](const Card item) {
      // A payable discard requirement does not make a K1 search Item live when the
      // inspected deck has no legal target:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
      if (item == Card::MysteriousTreasure) {
        return !deck_seen_ ||
            std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
      }
      if (item == Card::QuickBall) {
        return !deck_seen_ || std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);
      }
      if (item == Card::EarthenVessel) return earthen_vessel_has_legal_target();
      if (item == Card::UltraBall) return ultra_ball_has_legal_target();
      return false;
    };
    const auto one_discard_item_payable =
        [this, permit_payload_cost, &search_item_has_legal_target](const Card item) {
      if (hand_count(item) == 0 || !search_item_has_legal_target(item)) return false;
      // These Items require a card other than the played Item to pay their cost:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      return choose_discard(permit_payload_cost, true, true, item).has_value();
    };
    const auto ultra_ball_payable =
        [this, permit_payload_cost, &search_item_has_legal_target]() {
      if (hand_count(Card::UltraBall) == 0 ||
          !search_item_has_legal_target(Card::UltraBall)) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      const auto first = choose_discard(permit_payload_cost, true, true, Card::UltraBall);
      if (!first || !remove_one(state_.hand, *first)) {
        restore_hand();
        return false;
      }
      const auto second = choose_discard(can_play_payload_this_turn() && !is_payload(*first),
                                         true, true, Card::UltraBall);
      restore_hand();
      // Ultra Ball requires two other cards, so both selections must exist:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      return second.has_value();
    };
    const bool item_payload_outlet_in_hand = !item_locked() &&
        (hand_count(Card::BrilliantBlender) > 0 ||
         one_discard_item_payable(Card::MysteriousTreasure) ||
         one_discard_item_payable(Card::QuickBall) ||
         one_discard_item_payable(Card::EarthenVessel) ||
         ultra_ball_payable());
    // Item-lock effects prevent Item cards from being played from hand:
    // https://api.pokemontcg.io/v2/cards/me2pt5-16
    // A Supporter outlet is live only while the one Supporter play remains available:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool supporter_payload_outlet_in_hand = supporter_allowed() && hand_count(Card::ProfessorBurnet) > 0;
    const auto communication_recovery_return = [this]() -> std::optional<Card> {
      if (item_locked() || count_of(state_.discard, Card::PokemonCommunication) == 0) {
        return std::nullopt;
      }
      const bool missing_pokemon_connector = need_active_vstar() || need_energy() ||
          needs_tapu_connector() || needs_oricorio_connector();
      if (!missing_pokemon_connector) return std::nullopt;

      // Legacy Star can recover Pokémon Communication alone when a safe exchange
      // Pokémon is already held. Otherwise it can recover the Item and one Pokémon
      // as its two selected cards. The exchange plan is evaluated against the exact
      // post-recovery hand without inspecting hidden deck order:
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://api.pokemontcg.io/v2/cards/sm9-152
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
      // https://github.com/FlareZ123/pokemon-sims/issues/701
      if (pokemon_communication_plan(false, false, false)) {
        return Card::PokemonCommunication;
      }

      const std::vector<Card> original_hand = state_.hand;
      const std::vector<Card> original_discard = state_.discard;
      std::optional<Card> best_candidate;
      int best_score = 1000;
      for (const Card candidate : pokemon_communication_pokemon()) {
        if (count_of(original_discard, candidate) == 0) continue;
        state_.hand = original_hand;
        state_.discard = original_discard;
        if (!remove_one(state_.discard, candidate)) continue;
        state_.hand.push_back(candidate);
        const auto plan = pokemon_communication_plan(false, false, false);
        if (!plan || plan->returned != candidate) continue;
        const int score = pokemon_communication_return_score(candidate, false);
        if (score < best_score) {
          best_candidate = candidate;
          best_score = score;
        }
      }
      state_.hand = original_hand;
      state_.discard = original_discard;
      return best_candidate;
    };
    const bool known_communication_completion =
        communication_recovery_return().has_value();
    // Legacy Star can create the strict-JIT payload only when the public deck count is
    // nonzero and a modeled payload may remain. At K1 the identity check is exact; at
    // K0 it uses public fixed-list counts without inspecting hidden card identities:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    const bool only_random_payload_line = need_payload() && !state_.deck.empty() &&
        payload_might_be_in_deck() && !item_payload_outlet_in_hand && !supporter_payload_outlet_in_hand;

    // Legacy Star may recover any two cards from discard, so an already-ready payload
    // does not make the VSTAR Power dead when a known recovery immediately completes a
    // different setup axis: https://api.pokemontcg.io/v2/cards/swsh12-136
    // A recovered Basic Energy can be manually attached once during the turn. The
    // selected VSTAR may already be Active or have a legal same-turn Latias retreat path:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const Pokemon* energy_target = target_regi();
    const bool selected_vstar_can_be_active_this_turn = active_is_vstar() ||
        (need_active_vstar() && can_free_retreat_with_latias() && energy_target != nullptr &&
         energy_target->card == Card::RegidragoVstar);
    // Preserve the established Basic-Energy Legacy Star admission exactly for
    // canonical decks, then extend it with a DDE-only branch. This keeps the
    // temporary DDE modeling recipe from changing registered-deck decisions:
    // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement audit: https://github.com/FlareZ123/pokemon-sims/issues/2238
    const bool known_basic_energy_completion = payload_ready() &&
        selected_vstar_can_be_active_this_turn && need_energy() &&
        !state_.manual_energy_used &&
        ((grass_needed() > 0 && count_of(state_.discard, Card::Grass) > 0) ||
         (fire_needed() > 0 && count_of(state_.discard, Card::Fire) > 0));
    const bool known_dde_energy_completion = payload_ready() &&
        selected_vstar_can_be_active_this_turn && need_energy() &&
        !state_.manual_energy_used && energy_target != nullptr &&
        count_of(state_.discard, Card::DoubleDragonEnergy) > 0 && [&]() {
          const auto preferred = preferred_manual_energy_for(
              *energy_target, [this](const Card card) {
                return count_of(state_.discard, card) > 0;
              });
          if (!preferred || *preferred != Card::DoubleDragonEnergy) return false;
          Pokemon projected = *energy_target;
          return attach_energy_card(projected, *preferred) &&
              pays_apex_energy_cost(projected);
        }();
    const bool known_energy_completion =
        known_basic_energy_completion || known_dde_energy_completion;
    // A recovered Latias ex can be Benched, and Skyliner then gives the Basic Active no
    // Retreat Cost so a fully powered Benched VSTAR can be promoted this turn:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool known_latias_completion = payload_ready() && need_active_vstar() && !need_energy() &&
        state_.active && is_basic(state_.active->card) && bench_space() > 0 &&
        !in_play(Card::LatiasEx) && ability_available_for_pokemon(Card::LatiasEx) &&
        count_of(state_.discard, Card::LatiasEx) > 0;
    if (!only_random_payload_line && !known_energy_completion &&
        !known_latias_completion && !known_communication_completion) return false;

    state_.vstar_power_used = true;
    outcome_.used_legacy_star = true;
    std::vector<Card> discarded;
    for (int card = 0; card < 7 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      state_.discard.push_back(top);
      state_.discarded_this_turn.push_back(top);
      discarded.push_back(top);
    }
    std::vector<Card> recovered;
    const auto communication_return = communication_recovery_return();
    if (communication_return && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::PokemonCommunication)) {
        throw std::logic_error("Legacy Star Pokémon Communication target disappeared");
      }
      recovered.push_back(Card::PokemonCommunication);
      if (*communication_return != Card::PokemonCommunication) {
        if (!recover_discard_to_hand(*communication_return)) {
          throw std::logic_error("Legacy Star Pokémon Communication exchange target disappeared");
        }
        recovered.push_back(*communication_return);
      }
    }
    // Legacy Star may recover any card after discarding the top 7 cards:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool blender_payload_line = need_payload() && !item_locked() && payload_might_be_in_deck();
    if (blender_payload_line && count_of(state_.discard, Card::BrilliantBlender) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::BrilliantBlender)) throw std::logic_error("Legacy Star Blender target disappeared");
      recovered.push_back(Card::BrilliantBlender);
    }
    const bool burnet_payload_line = need_payload() && supporter_allowed() && payload_might_be_in_deck();
    if (burnet_payload_line && count_of(state_.discard, Card::ProfessorBurnet) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::ProfessorBurnet)) throw std::logic_error("Legacy Star Burnet target disappeared");
      recovered.push_back(Card::ProfessorBurnet);
    }

    if (legacy_star_delayed_vessel_route() && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Legacy Star delayed Earthen Vessel target disappeared");
      }
      recovered.push_back(Card::EarthenVessel);
    }

    // K1 proves a complete Earthen Vessel -> Mysterious Treasure -> Quick
    // Ball finish when Legacy Star recovers both Items. Vessel spends the
    // now-inert Steven's Resolve and searches two Grass Energy, Treasure
    // spends one Grass while preserving the other manual attachment, and
    // Quick Ball discards the searched Dragon payload:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Core Item, Supporter, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1576
    const bool issue_1576_legacy_star_payload_bridge =
        recovered.empty() && scenario_.dci == DciProfile::NoDiscardControl &&
        state_.turn == 2 && !item_locked() && deck_seen_ &&
        state_.supporter_used && !state_.manual_energy_used &&
        active_is_vstar() && grass_needed() == 1 && fire_needed() == 0 &&
        need_payload() && hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::StevensResolve) > 0 &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
        count_of(state_.discard, Card::QuickBall) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2;
    if (issue_1576_legacy_star_payload_bridge) {
      std::vector<Card> projected_deck = state_.deck;
      const auto erase_one = [&projected_deck](const Card card) {
        const auto it = std::find(
            projected_deck.begin(), projected_deck.end(), card);
        if (it == projected_deck.end()) return false;
        projected_deck.erase(it);
        return true;
      };
      const bool energy_targets_exist =
          erase_one(Card::Grass) && erase_one(Card::Grass);
      const auto payload = std::find_if(
          projected_deck.begin(), projected_deck.end(),
          [](const Card card) {
            return is_payload(card) && is_dragon_or_psychic(card);
          });
      const bool payload_target_exists = payload != projected_deck.end();
      if (payload_target_exists) projected_deck.erase(payload);
      const bool distinct_basic_target_exists = std::any_of(
          projected_deck.begin(), projected_deck.end(), is_basic);
      if (energy_targets_exist && payload_target_exists &&
          distinct_basic_target_exists) {
        if (!recover_discard_to_hand(Card::MysteriousTreasure)) {
          throw std::logic_error(
              "Legacy Star Mysterious Treasure bridge target disappeared");
        }
        if (!recover_discard_to_hand(Card::QuickBall)) {
          throw std::logic_error(
              "Legacy Star Quick Ball bridge target disappeared");
        }
        recovered.push_back(Card::MysteriousTreasure);
        recovered.push_back(Card::QuickBall);
      }
    }

    // Legacy Star may return Latias ex from discard, and Skyliner then gives the
    // Basic Active no Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-76
    const bool latias_retreat_line = need_active_vstar() && state_.active && is_basic(state_.active->card) &&
        bench_space() > 0 && !in_play(Card::LatiasEx) && ability_available_for_pokemon(Card::LatiasEx);
    if (latias_retreat_line && count_of(state_.discard, Card::LatiasEx) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::LatiasEx)) throw std::logic_error("Legacy Star Latias target disappeared");
      recovered.push_back(Card::LatiasEx);
    }

    // Evolution Incense plus a hand-discard outlet can form a two-card payload bridge:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    const bool evolution_incense_payload_line = need_payload() && !item_locked() && payload_might_be_in_deck() &&
        count_of(state_.discard, Card::EvolutionIncense) > 0 && recovered.empty();
    if (evolution_incense_payload_line) {
      // Evaluate the exact deck after Evolution Incense removes its first available
      // modeled Evolution Dragon payload. At K0, one remaining card preserves public
      // fixed-list plausibility; at K1, the promised second Item needs its own target:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
      const auto post_incense_search_has_target = [this](const Card outlet) {
        if (state_.deck.size() <= 1U) return false;
        if (!deck_seen_) return true;

        std::vector<Card> remaining = state_.deck;
        auto payload = remaining.end();
        // Evolution Incense can fetch Appletun after Legacy Star recovers the
        // two-card search-and-discard bridge:
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/swsh1-163
        // https://api.pokemontcg.io/v2/cards/sv8-140
        // https://github.com/FlareZ123/pokemon-sims/issues/1255
        for (const Card candidate :
             {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
              Card::Appletun}) {
          payload = std::find(remaining.begin(), remaining.end(), candidate);
          if (payload != remaining.end()) break;
        }
        if (payload == remaining.end()) return false;
        remaining.erase(payload);

        if (outlet == Card::MysteriousTreasure) {
          return std::any_of(remaining.begin(), remaining.end(), is_dragon_or_psychic);
        }
        if (outlet == Card::QuickBall) {
          return std::any_of(remaining.begin(), remaining.end(), is_basic);
        }
        if (outlet == Card::EarthenVessel) {
          return std::any_of(remaining.begin(), remaining.end(), [](const Card card) {
            return card == Card::Grass || card == Card::Fire;
          });
        }
        if (outlet == Card::UltraBall) {
          return std::any_of(remaining.begin(), remaining.end(), is_pokemon);
        }
        return false;
      };

      // Ultra Ball can follow Evolution Incense only when the fetched Dragon plus a
      // distinct policy-admissible hand card can pay its two-other-card cost:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      if (post_incense_search_has_target(Card::UltraBall) &&
          count_of(state_.discard, Card::UltraBall) > 0) {
        const std::vector<Card> base_hand = state_.hand;
        const std::vector<Card> base_discard = state_.discard;
        const std::vector<Card> base_discarded_this_turn = state_.discarded_this_turn;
        const auto restore = [this, &base_hand, &base_discard, &base_discarded_this_turn]() {
          state_.hand = base_hand;
          state_.discard = base_discard;
          state_.discarded_this_turn = base_discarded_this_turn;
        };

        bool ultra_bridge_payable = false;
        if (recover_discard_to_hand(Card::EvolutionIncense) &&
            recover_discard_to_hand(Card::UltraBall)) {
          // Simulate the exact post-Incense hand: the recovered Incense has been
          // played, the searched Dragon is present, and Ultra Ball reserves itself.
          // A distinct held Incense may then be Ultra Ball's second cost only when
          // the Dragon payload completes the sole missing setup axis:
          // https://api.pokemontcg.io/v2/cards/swsh1-163
          // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
          // https://api.pokemontcg.io/v2/cards/swsh12-136
          if (!remove_one(state_.hand, Card::EvolutionIncense)) {
            throw std::logic_error("Legacy Star simulated Incense play disappeared");
          }
          state_.hand.push_back(Card::MegaDragonite);
          if (!remove_one(state_.hand, Card::MegaDragonite)) {
            throw std::logic_error("Legacy Star representative payload disappeared");
          }
          const bool payload_is_only_missing_axis = need_payload() && !need_regi() &&
              !need_vstar() && !need_energy() && !need_active_vstar();
          ultra_bridge_payable =
              choose_discard(false, true, true, Card::UltraBall).has_value() ||
              (payload_is_only_missing_axis &&
               hand_count(Card::EvolutionIncense) > 0);
        }
        restore();

        if (ultra_bridge_payable) {
          if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
          if (!recover_discard_to_hand(Card::UltraBall)) throw std::logic_error("Legacy Star Ultra Ball target disappeared");
          recovered.push_back(Card::EvolutionIncense);
          recovered.push_back(Card::UltraBall);
        }
      }

      if (recovered.empty()) {
        for (const Card outlet : {Card::MysteriousTreasure, Card::QuickBall, Card::EarthenVessel}) {
          if (count_of(state_.discard, outlet) == 0 ||
              !post_incense_search_has_target(outlet)) continue;
          if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
          if (!recover_discard_to_hand(outlet)) throw std::logic_error("Legacy Star hand-discard outlet disappeared");
          recovered.push_back(Card::EvolutionIncense);
          recovered.push_back(outlet);
          break;
        }
      }

      // Serena's first mode requires discarding at least one hand card before drawing
      // to five. After Incense fetches the Dragon, that payload is a guaranteed legal
      // discard while the one Supporter play remains open:
      // https://api.pokemontcg.io/v2/cards/swsh12-164
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      if (recovered.empty() && supporter_allowed() && count_of(state_.discard, Card::Serena) > 0) {
        if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
        if (!recover_discard_to_hand(Card::Serena)) throw std::logic_error("Legacy Star Serena target disappeared");
        recovered.push_back(Card::EvolutionIncense);
        recovered.push_back(Card::Serena);
      }
    }

    // Earthen Vessel can turn one or two Legacy Star recoveries into the exact Basic
    // Energy needed for the unused manual attachment. Recover Vessel alone when the
    // hand already supplies its discard cost, or recover a second policy-admissible
    // cost card from the newly discarded cards: https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // A permitted Dragon payload is the strongest strict-JIT Vessel cost because the
    // required discard also establishes Apex Dragon's current-turn payload axis:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool needed_energy_might_remain =
        (grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
        (fire_needed() > 0 && might_be_unseen(Card::Fire));
    const bool immediate_vessel_energy_line = need_energy() &&
        !state_.manual_energy_used && needed_energy_might_remain;
    // When Powerglass is the exact end-of-turn Energy completion, Legacy Star can
    // preserve Earthen Vessel for the first later attack turn. Vessel must retain a
    // legal Basic Energy target after the seven-card discard:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    const bool delayed_vessel_payload_line =
        hold_payload_outlet_for_post_powerglass_turn() && earthen_vessel_has_legal_target();
    if (recovered.size() < 2U && !item_locked() &&
        (immediate_vessel_energy_line || delayed_vessel_payload_line) &&
        count_of(state_.discard, Card::EarthenVessel) > 0) {
      const std::vector<Card> base_hand = state_.hand;
      const std::vector<Card> base_discard = state_.discard;
      const std::vector<Card> base_discarded_this_turn = state_.discarded_this_turn;
      const auto restore = [this, &base_hand, &base_discard, &base_discarded_this_turn]() {
        state_.hand = base_hand;
        state_.discard = base_discard;
        state_.discarded_this_turn = base_discarded_this_turn;
      };

      bool found_vessel_bridge = false;
      if (recover_discard_to_hand(Card::EarthenVessel) &&
          choose_discard(can_play_payload_this_turn(), true, true, Card::EarthenVessel).has_value()) {
        recovered.push_back(Card::EarthenVessel);
        found_vessel_bridge = true;
      } else {
        restore();
      }

      if (!found_vessel_bridge && recovered.empty()) {
        for (const Card candidate : base_discard) {
          restore();
          if (!recover_discard_to_hand(Card::EarthenVessel) || !recover_discard_to_hand(candidate)) continue;
          if (!choose_discard(can_play_payload_this_turn(), true, true, Card::EarthenVessel).has_value()) continue;
          recovered.push_back(Card::EarthenVessel);
          recovered.push_back(candidate);
          found_vessel_bridge = true;
          break;
        }
      }
      if (!found_vessel_bridge) restore();
    }

    // Legacy Star may recover any two cards. DDE is a legal Special Energy recovery
    // and is selected ahead of Basic Energy only when the shared attachment policy
    // proves its two flexible units are strictly stronger for the selected Dragon:
    // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Enhancement audit: https://github.com/FlareZ123/pokemon-sims/issues/2238
    if (recovered.size() < 2U && !state_.manual_energy_used && need_energy() &&
        count_of(state_.discard, Card::DoubleDragonEnergy) > 0) {
      const Pokemon* target = target_regi();
      if (target != nullptr) {
        const auto preferred = preferred_manual_energy_for(
            *target, [this](const Card card) {
              return count_of(state_.discard, card) > 0;
            });
        if (preferred && *preferred == Card::DoubleDragonEnergy) {
          if (!recover_discard_to_hand(Card::DoubleDragonEnergy)) {
            throw std::logic_error("Legacy Star DDE target disappeared");
          }
          recovered.push_back(Card::DoubleDragonEnergy);
        }
      }
    }

    // Recover Basic Energy for a live manual attachment. Crispin and Arven cannot
    // resolve after a spent Supporter slot, though either remains a valid future-turn
    // fallback when no payable same-turn recovery bridge above exists:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://www.pokemon.com/us/pokemon-tcg/rules
    for (const Card desired : {Card::Grass, Card::Fire, Card::Crispin, Card::Arven}) {
      if (recovered.size() >= 2U) break;
      if (is_payload(desired) && strict_payload_timing()) continue;
      if ((desired == Card::Grass || desired == Card::Fire) && state_.manual_energy_used) continue;
      const auto it = std::find(state_.discard.begin(), state_.discard.end(), desired);
      if (it == state_.discard.end()) continue;
      if (desired == Card::Grass && grass_needed() == 0) continue;
      if (desired == Card::Fire && fire_needed() == 0) continue;
      if (!recover_discard_to_hand(desired)) throw std::logic_error("Legacy Star recovery target disappeared");
      recovered.push_back(desired);
    }
    trace("LEGACY STAR", "R-RVS-01; R-VSTAR-01", "Discarded top 7: " + join_cards(discarded) + "; recovered: " + join_cards(recovered) + ".");
    return true;
  }
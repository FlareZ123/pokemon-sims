  struct PokemonCommunicationPlan {
    Card returned;
    Card target;
  };

  static constexpr std::array<Card, 11> pokemon_communication_pokemon() {
    return {Card::RegidragoV, Card::RegidragoVstar, Card::Dragapult,
            Card::MegaDragonite, Card::DialgaGX, Card::GoodraVstar,
            Card::TapuLeleGX, Card::LatiasEx, Card::MawileGX,
            Card::Oricorio, Card::Dipplin};
  }

  bool pokemon_communication_has_exchange_candidate() const {
    return std::any_of(state_.hand.begin(), state_.hand.end(), is_pokemon);
  }

  bool pokemon_communication_opening_route_has_safe_exchange() const {
    if (item_locked() || hand_count(Card::PokemonCommunication) == 0 ||
        !might_be_unseen(Card::RegidragoV)) {
      return false;
    }
    // Before an opening Active is selected, Tapu Lele-GX and Oricorio are both
    // scarce on-play connectors. Do not call Pokémon Communication a redundant
    // search route when one of those two would be its only possible exchange. A
    // Dipplin or duplicated Pokémon is a genuinely lower-marginal third card:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    for (const Card card : pokemon_communication_pokemon()) {
      if (card == Card::TapuLeleGX || card == Card::Oricorio) continue;
      if (hand_count(card) > 1 || (card == Card::Dipplin && hand_count(card) > 0)) {
        return true;
      }
    }
    return false;
  }

  bool pokemon_communication_has_direct_target_route(const Card target) {
    if (item_locked() || hand_count(Card::PokemonCommunication) == 0 ||
        !is_pokemon(target)) {
      return false;
    }
    for (const Card returned : pokemon_communication_pokemon()) {
      if (returned == target || returned == Card::TapuLeleGX ||
          hand_count(returned) == 0 ||
          !pokemon_communication_target_available_after_return(target, returned)) {
        continue;
      }
      // Direct Item routing must preserve a live Wonder Tag Pokémon. Passing true
      // for the Supporter-used scoring mode prevents recursive Tapu valuation while
      // the explicit skip above protects the card itself:
      // https://api.pokemontcg.io/v2/cards/sm9-152
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://github.com/FlareZ123/pokemon-sims/issues/701
      if (pokemon_communication_return_score(returned, true) < 800) return true;
    }
    return false;
  }

  std::optional<PokemonCommunicationPlan>
  pokemon_communication_plan_after_playing_supporter(
      const Card supporter, const bool communication_is_held) const {
    Engine simulated = *this;
    if (!remove_one(simulated.state_.hand, supporter) &&
        !simulated.state_.supporter_used) {
      return std::nullopt;
    }
    simulated.state_.supporter_used = true;
    return simulated.pokemon_communication_plan(
        true, communication_is_held, true);
  }

  std::optional<PokemonCommunicationPlan>
  pokemon_communication_plan_after_wonder_tag_arven() const {
    Engine simulated = *this;
    if (simulated.bench_space() == 0 ||
        !remove_one(simulated.state_.hand, Card::TapuLeleGX)) {
      return std::nullopt;
    }
    simulated.state_.bench.push_back(Pokemon{Card::TapuLeleGX, state_.turn});
    simulated.state_.supporter_used = true;
    return simulated.pokemon_communication_plan(true, false, true);
  }

  int pokemon_communication_return_score(const Card card,
                                         const bool supporter_will_be_used) {
    if (!is_pokemon(card)) return 1000;
    if (hand_count(card) > 1) return 5;
    if (card == Card::Dipplin) return 10;
    if (card == Card::MawileGX) return 20;
    if (card == Card::RegidragoV) {
      return need_regi() && in_play_count(Card::RegidragoV) == 0 &&
                     in_play_count(Card::RegidragoVstar) == 0
                 ? 900
                 : 30;
    }
    if (card == Card::RegidragoVstar) {
      return need_vstar() && !has_vstar() ? 900 : 30;
    }
    if (card == Card::LatiasEx) {
      return need_active_vstar() && !in_play(Card::LatiasEx) &&
                     ability_available_for_pokemon(Card::LatiasEx)
                 ? 900
                 : 35;
    }
    if (card == Card::TapuLeleGX) {
      return !supporter_will_be_used && !in_play(Card::TapuLeleGX) &&
                     ability_available_for_pokemon(Card::TapuLeleGX) &&
                     needs_tapu_connector()
                 ? 900
                 : 40;
    }
    if (card == Card::Oricorio) {
      return !in_play(Card::Oricorio) && needs_oricorio_connector() ? 900 : 40;
    }
    if (is_payload(card)) {
      return need_payload() && can_play_payload_this_turn() ? 900 : 50;
    }
    return 100;
  }

  bool pokemon_communication_target_available_after_return(const Card target,
                                                            const Card returned) const {
    if (!is_pokemon(target)) return false;
    if (target == returned) return true;
    return might_be_unseen(target);
  }

  bool pokemon_communication_payload_outlet_after_exchange(
      const Card returned, const Card fetched,
      const bool communication_is_held,
      const bool supporter_will_be_used) {
    const std::vector<Card> original_hand = state_.hand;
    const auto restore = [this, &original_hand]() { state_.hand = original_hand; };
    if (!communication_is_held) state_.hand.push_back(Card::PokemonCommunication);
    if (!remove_one(state_.hand, Card::PokemonCommunication) ||
        !remove_one(state_.hand, returned)) {
      restore();
      return false;
    }
    state_.hand.push_back(fetched);

    // Pokémon Communication returns one Pokémon before the later search. That
    // returned card is a guaranteed target for a subsequent search Item only when it
    // belongs to that Item's printed target class:
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    const bool mysterious_target = is_dragon_or_psychic(returned) ||
        std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
          return is_dragon_or_psychic(entry.first) &&
                 might_be_unseen(entry.first);
        });
    const bool quick_target = is_basic(returned) ||
        std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
          return is_basic(entry.first) && might_be_unseen(entry.first);
        });
    const bool vessel_target = might_be_unseen(Card::Grass) ||
                               might_be_unseen(Card::Fire);

    const auto one_discard_item_payable = [this](const Card item) {
      return hand_count(item) > 0 &&
             choose_discard(true, true, true, item).has_value();
    };
    const bool mysterious_route = mysterious_target &&
        one_discard_item_payable(Card::MysteriousTreasure);
    const bool quick_route = quick_target && one_discard_item_payable(Card::QuickBall);
    const bool vessel_route = vessel_target && one_discard_item_payable(Card::EarthenVessel);

    bool ultra_route = false;
    if (hand_count(Card::UltraBall) > 0) {
      const auto first = choose_discard(true, true, true, Card::UltraBall);
      if (first && remove_one(state_.hand, *first)) {
        ultra_route = choose_discard(
            can_play_payload_this_turn() && !is_payload(*first), true, true,
            Card::UltraBall).has_value();
      }
    }
    const bool serena_route = !supporter_will_be_used && supporter_allowed() &&
                              hand_count(Card::Serena) > 0;
    restore();
    return mysterious_route || quick_route || vessel_route || ultra_route ||
           serena_route;
  }

  std::optional<PokemonCommunicationPlan> pokemon_communication_plan(
      const bool permit_payload,
      const bool communication_is_held = true,
      const bool supporter_will_be_used = false) {
    if (item_locked()) return std::nullopt;
    if (communication_is_held && hand_count(Card::PokemonCommunication) == 0) {
      return std::nullopt;
    }

    std::vector<Card> targets;
    const auto add_target = [&targets](const Card target) {
      if (std::find(targets.begin(), targets.end(), target) == targets.end()) {
        targets.push_back(target);
      }
    };
    if (need_regi() && bench_space() > 0 && hand_count(Card::RegidragoV) == 0) {
      add_target(Card::RegidragoV);
    }
    if (need_vstar() && hand_count(Card::RegidragoVstar) == 0) {
      add_target(Card::RegidragoVstar);
    }
    if (need_active_vstar() && bench_space() > 0 && !in_play(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 &&
        ability_available_for_pokemon(Card::LatiasEx)) {
      add_target(Card::LatiasEx);
    }
    if (need_energy() && bench_space() > 0 && !in_play(Card::Oricorio) &&
        hand_count(Card::Oricorio) == 0 && needs_oricorio_connector()) {
      add_target(Card::Oricorio);
    }
    if (!supporter_will_be_used && bench_space() > 0 &&
        !in_play(Card::TapuLeleGX) && hand_count(Card::TapuLeleGX) == 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        needs_tapu_connector()) {
      add_target(Card::TapuLeleGX);
    }

    const bool payload_already_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (permit_payload && need_payload() && can_play_payload_this_turn() &&
        !payload_already_held) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX}) {
        add_target(payload);
      }
    }

    for (const Card target : targets) {
      std::optional<PokemonCommunicationPlan> best;
      int best_score = 1000;
      for (const Card returned : pokemon_communication_pokemon()) {
        if (returned == target || hand_count(returned) == 0) continue;
        if (!pokemon_communication_target_available_after_return(target, returned)) {
          continue;
        }
        const int score = pokemon_communication_return_score(
            returned, supporter_will_be_used);
        if (score >= 800 || score >= best_score) continue;
        if (is_payload(target) &&
            !pokemon_communication_payload_outlet_after_exchange(
                returned, target, communication_is_held,
                supporter_will_be_used)) {
          continue;
        }
        best = PokemonCommunicationPlan{returned, target};
        best_score = score;
      }
      if (best) return best;
    }
    return std::nullopt;
  }

  bool pokemon_communication_can_find_after_recovery(
      const Card target, const Card supporter_leaving_hand,
      const std::initializer_list<Card> restored_cards,
      const bool permit_payload = false) const {
    if (item_locked() || hand_count(Card::PokemonCommunication) == 0) return false;

    Engine simulated = *this;
    if (!remove_one(simulated.state_.hand, supporter_leaving_hand)) return false;
    for (const Card restored : restored_cards) simulated.state_.deck.push_back(restored);

    // Recovery Supporters shuffle their selected Pokémon into the deck before the
    // held Item resolves. Evaluate the exact post-Supporter hand and restored deck,
    // rather than treating Pokémon Communication's presence as a universal search:
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
    // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
    // Core search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Regression scope: https://github.com/FlareZ123/pokemon-sims/issues/701
    const auto plan = simulated.pokemon_communication_plan(
        permit_payload, true, true);
    return plan && plan->target == target;
  }

  Card pokemon_communication_target_after_search_started(
      const Card preferred, const Card returned) {
    const auto present = [this](const Card card) {
      return deck_count_after_search_started(card) > 0;
    };
    if (present(preferred)) return preferred;

    // The preferred K0 target can prove absent only after the legal search starts.
    // Continue the same resolution with the strongest actually present Pokémon
    // rather than leaking hidden Prize information by declining the Item before it
    // is played. The returned Pokémon guarantees at least one legal target:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    if (need_regi() && bench_space() > 0 && present(Card::RegidragoV)) {
      return Card::RegidragoV;
    }
    if (need_vstar() && present(Card::RegidragoVstar)) {
      return Card::RegidragoVstar;
    }
    if (need_active_vstar() && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        present(Card::LatiasEx)) {
      return Card::LatiasEx;
    }
    if (need_energy() && bench_space() > 0 &&
        ability_available_for_pokemon(Card::Oricorio) &&
        needs_oricorio_connector() && present(Card::Oricorio)) {
      return Card::Oricorio;
    }
    if (!state_.supporter_used && bench_space() > 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        needs_tapu_connector() && present(Card::TapuLeleGX)) {
      return Card::TapuLeleGX;
    }
    if (is_payload(preferred)) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX}) {
        if (present(payload)) return payload;
      }
    }
    if (present(returned)) return returned;
    for (const Card pokemon : pokemon_communication_pokemon()) {
      if (present(pokemon)) return pokemon;
    }
    throw std::logic_error("Pokémon Communication lost its guaranteed returned target");
  }

  bool resolve_pokemon_communication(const Card returned, const Card target) {
    if (item_locked() || hand_count(Card::PokemonCommunication) == 0 ||
        !is_pokemon(returned) || !is_pokemon(target) ||
        hand_count(returned) == 0) {
      return false;
    }

    // The revealed hand Pokémon enters the deck before the search. It is therefore
    // a legal target of the same resolving Pokémon Communication, including when the
    // deck was empty before the exchange:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!remove_one(state_.hand, Card::PokemonCommunication) ||
        !remove_one(state_.hand, returned)) {
      throw std::logic_error("Pokémon Communication preflight diverged from hand state");
    }
    state_.discard.push_back(Card::PokemonCommunication);
    state_.deck.push_back(returned);
    record_deck_search_knowledge("Pokémon Communication");
    const Card actual_target =
        pokemon_communication_target_after_search_started(target, returned);
    if (!move_deck_to_hand(actual_target)) {
      throw std::logic_error("Pokémon Communication target disappeared after search started");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-PCOMM-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          "Returned " + std::string(name(returned)) +
              " to the deck and searched " + std::string(name(actual_target)) + ".");
    return true;
  }

  bool play_pokemon_communication(const bool permit_payload = false) {
    const auto plan = pokemon_communication_plan(permit_payload);
    if (!plan) return false;
    return resolve_pokemon_communication(plan->returned, plan->target);
  }

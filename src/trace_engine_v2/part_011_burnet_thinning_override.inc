  bool play_professor_burnet() {
    // Professor Burnet may search for and discard up to 2 deck cards:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck()) {
      return false;
    }

    // Strict JIT recognizes only a Dragon discarded during the same turn that the
    // Active Regidrago VSTAR reaches GGF. Burnet consumes the Supporter play, so do
    // not spend it when an observable non-payload axis cannot still finish this turn:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/719
    if (strict_payload_timing() && has_vstar()) {
      const bool held_latias_route = state_.active && is_basic(state_.active->card) &&
          hand_count(Card::LatiasEx) > 0 && bench_space() > 0 &&
          ability_available_for_pokemon(Card::LatiasEx) && !state_.retreat_used;
      const bool direct_active_evolution = state_.active &&
          state_.active->card == Card::RegidragoV &&
          state_.active->entered_turn < state_.turn &&
          (hand_count(Card::RegidragoVstar) > 0 ||
           held_item_can_find_pending_vstar());
      const bool active_axis_without_new_bench = !need_active_vstar() ||
          can_free_retreat_with_latias() || direct_active_evolution;
      const bool active_axis_can_finish = active_axis_without_new_bench ||
          held_latias_route;
      if (!active_axis_can_finish) return false;

      const int missing_energy = grass_needed() + fire_needed();
      if (missing_energy > 0) {
        if (missing_energy != 1 || state_.manual_energy_used) return false;

        const Card needed_energy = grass_needed() == 1 ? Card::Grass : Card::Fire;
        const bool held_energy_route = hand_count(needed_energy) > 0;
        const bool needed_energy_might_be_in_deck = might_be_unseen(needed_energy);
        const bool held_oricorio_route = needed_energy_might_be_in_deck &&
            hand_count(Card::Oricorio) > 0 && !in_play(Card::Oricorio) &&
            bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio);
        const bool held_vessel_route = needed_energy_might_be_in_deck &&
            !item_locked() && hand_count(Card::EarthenVessel) > 0 &&
            choose_discard(false, true, true, Card::EarthenVessel).has_value();
        const bool energy_axis_without_new_bench =
            held_energy_route || held_vessel_route;
        if (!energy_axis_without_new_bench && !held_oricorio_route) return false;
        if (!active_axis_without_new_bench && held_latias_route &&
            !energy_axis_without_new_bench && held_oricorio_route &&
            bench_space() < 2) {
          return false;
        }
      }
    }

    remove_one(state_.hand, Card::ProfessorBurnet);
    state_.discard.push_back(Card::ProfessorBurnet);
    state_.supporter_used = true;
    std::vector<Card> discarded;
    // Apex Dragon copies attacks from Dragon Pokémon in discard. Fill Burnet's two
    // selections with modeled A/S payloads before considering Dipplin or dead cards:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh11-136
    // https://api.pokemontcg.io/v2/cards/sm5-100
    for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                            Card::GoodraVstar, Card::DialgaGX}) {
      if (discarded.size() >= 2U) break;
      if (move_deck_to_discard(card)) discarded.push_back(card);
    }
    if (discarded.size() < 2U && move_deck_to_discard(Card::Dipplin)) {
      discarded.push_back(Card::Dipplin);
    }

    // Celestial Roar processes the top 3 cards and attaches every Basic Energy found.
    // In no-discard-control, remove one no-lock setup-dead card when Burnet's second
    // legal selection strictly improves that immediate random route:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    const bool active_can_use_celestial_roar =
        scenario_.dci == DciProfile::NoDiscardControl &&
        scenario_.locks == LockMode::None && !first_turn_going_first() &&
        !state_.turn_ended && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass + state_.active->fire >= 1 &&
        (state_.active->grass < 2 || state_.active->fire < 1);
    const bool needed_energy_remains =
        state_.active &&
        ((state_.active->grass < 2 && might_be_unseen(Card::Grass)) ||
         (state_.active->fire < 1 && might_be_unseen(Card::Fire)));
    if (discarded.size() < 2U && active_can_use_celestial_roar &&
        needed_energy_remains && state_.deck.size() > 3U) {
      for (const Card card : {Card::PathToPeak, Card::MawileGX, Card::Guzma,
                              Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell}) {
        if (move_deck_to_discard(card)) {
          discarded.push_back(card);
          break;
        }
      }
    }

    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-BURNET-01; R-GAME-SUPPORTER; P-DCI-01",
          "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

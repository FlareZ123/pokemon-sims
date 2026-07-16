  bool strict_jit_burnet_non_payload_axes_can_finish_this_turn() const {
    if (!strict_payload_timing()) return true;

    // Professor Burnet consumes the turn's Supporter play and only places deck cards
    // in discard. It cannot reopen the once-per-turn manual attachment window. Under
    // strict JIT, do not spend Burnet after that window is closed while the selected
    // Regidrago attacker still lacks part of Apex Dragon's GGF attack cost:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/719
    if (need_energy() && state_.manual_energy_used) return false;

    if (!need_active_vstar()) return true;

    // Burnet also consumes the Supporter slot that Tate & Liza would need to promote
    // a Benched Regidrago VSTAR. Keep the payload play only when Latias ex already
    // supplies a legal free-retreat route, or when the current Active Regidrago V can
    // evolve from the held VSTAR later in this same turn:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/719
    const bool active_regi_can_evolve_from_hand = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0;
    return can_free_retreat_with_latias() || active_regi_can_evolve_from_hand;
  }

  bool play_professor_burnet() {
    // Professor Burnet may search for and discard up to 2 deck cards:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck() ||
        !strict_jit_burnet_non_payload_axes_can_finish_this_turn()) {
      return false;
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

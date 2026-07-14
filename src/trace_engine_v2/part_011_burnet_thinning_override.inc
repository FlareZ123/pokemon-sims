  bool play_professor_burnet() {
    // Professor Burnet may search for and discard up to 2 deck cards:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck()) {
      return false;
    }

    remove_one(state_.hand, Card::ProfessorBurnet);
    state_.discard.push_back(Card::ProfessorBurnet);
    state_.supporter_used = true;
    std::vector<Card> discarded;
    for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                            Card::GoodraVstar, Card::DialgaGX}) {
      if (move_deck_to_discard(card)) {
        discarded.push_back(card);
        break;
      }
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

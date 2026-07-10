  bool use_celestial_roar() {
    if (first_turn_going_first() || state_.turn_ended || !state_.active ||
        state_.active->card != Card::RegidragoV) {
      return false;
    }

    // Celestial Roar has no turn-number restriction. The core opening-turn rule only
    // prevents the player going first from attacking on their first turn:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // The setup policy attacks only while this Active Regidrago V is still missing
    // part of GGF, avoiding an optional discard-3 that cannot advance its Energy axis.
    const bool active_needs_energy = state_.active->grass < 2 || state_.active->fire < 1;
    if (!active_needs_energy) return false;

    // Celestial Roar costs one Colorless Energy, so any attached Energy pays it:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    if (state_.active->grass + state_.active->fire < 1) return false;

    // Celestial Roar can advance GGF only when a currently needed Energy type may
    // remain in the deck. After K1, might_be_unseen() uses the inspected deck exactly;
    // before K1, it uses only public fixed-list counts:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    const bool needed_energy_might_be_in_deck =
        (state_.active->grass < 2 && might_be_unseen(Card::Grass)) ||
        (state_.active->fire < 1 && might_be_unseen(Card::Fire));
    // No-discard-control may still use the random top-three effect to bank a missing
    // Dragon payload. Strict and matchup-flex JIT require the payload to enter discard
    // during the later ready turn, so an attack by unevolved Regidrago V cannot advance
    // that axis for them:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    const bool payload_banking_might_advance = !strict_payload_timing() && need_payload() &&
                                               payload_might_be_in_deck();
    if (!needed_energy_might_be_in_deck && !payload_banking_might_advance) {
      trace("HOLD ATTACK", "R-RV-01; P-KNOWLEDGE-01; P-JIT-01",
            "Celestial Roar cannot attach a needed Energy or bank a permitted payload from the known remaining deck.");
      return false;
    }

    std::vector<Card> processed;
    std::vector<Card> attached;
    std::vector<Card> discarded;
    for (int card = 0; card < 3 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      processed.push_back(top);
      if (top == Card::Grass) {
        ++state_.active->grass;
        attached.push_back(top);
      } else if (top == Card::Fire) {
        ++state_.active->fire;
        attached.push_back(top);
      } else {
        state_.discard.push_back(top);
        state_.discarded_this_turn.push_back(top);
        discarded.push_back(top);
      }
    }
    state_.turn_ended = true;
    trace("ATTACK", "R-RV-01; R-GAME-ATTACK",
          "Regidrago V used Celestial Roar; processed top 3: " + join_cards(processed) +
          "; attached Energy: " + join_cards(attached) +
          "; remaining discarded: " + join_cards(discarded) + ".");
    return true;
  }

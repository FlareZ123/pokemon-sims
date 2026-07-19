  bool use_celestial_roar() {
    if (first_turn_going_first() || state_.turn_ended || !state_.active ||
        state_.active->card != Card::RegidragoV) {
      return false;
    }

    // Celestial Roar has no turn-number restriction. The core opening-turn rule only
    // prevents the player going first from attacking on their first turn:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // The setup policy attacks only when Celestial Roar may advance either the
    // Active's GGF Energy axis or a permitted no-discard-control payload bank:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    const bool active_needs_energy = missing_grass + missing_fire > 0;

    // Celestial Roar costs one Colorless Energy, so any attached Energy pays it:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    if (state_.active->grass + state_.active->fire < 1) return false;

    const bool held_dragon_payload =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool held_supporter_payload_route =
        (hand_count(Card::Serena) > 0 && held_dragon_payload) ||
        (deck_seen_ && hand_count(Card::ProfessorBurnet) > 0 &&
         std::any_of(state_.deck.begin(), state_.deck.end(), is_payload));
    const bool held_energy_and_payload_route_guarantee_next_window =
        strict_payload_timing() && state_.manual_energy_used &&
        state_.turn < scenario_.max_turn && missing_grass + missing_fire == 1 &&
        ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||
         (missing_fire == 1 && hand_count(Card::Fire) > 0)) &&
        held_supporter_payload_route;
    if (held_energy_and_payload_route_guarantee_next_window) {
      // The unevolved Regidrago V cannot become Regidrago VSTAR before a later legal
      // evolution window. One held Basic Energy already guarantees GGF through that
      // turn's manual attachment, while held Serena plus a Dragon or held Professor
      // Burnet plus a known deck Dragon guarantees a legal strict-JIT payload route.
      // Celestial Roar cannot improve either completed axis and can randomly remove the
      // unresolved VSTAR or connector, so preserve those resources:
      // Regidrago V and Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Regidrago VSTAR evolution and Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Manual attachment, evolution, attack, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Earliest complete route and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1079
      trace("HOLD ATTACK", "R-RV-01; R-GAME-ENERGY; R-SERENA-01; R-BURNET-01; P-JIT-01; P-COMPRESS-01",
            "Held Energy and a held Supporter payload route already cover the next legal ready window.");
      return false;
    }

    // Celestial Roar can advance GGF only when a currently needed Energy type may
    // remain in the deck. After K1, might_be_unseen() uses the inspected deck exactly;
    // before K1, it uses only public fixed-list counts:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    const bool needed_energy_might_be_in_deck = active_needs_energy &&
        ((state_.active->grass < 2 && might_be_unseen(Card::Grass)) ||
         (state_.active->fire < 1 && might_be_unseen(Card::Fire)));
    // No-discard-control may still use the random top-three effect to bank a missing
    // Dragon payload. Strict and matchup-flex JIT require the payload to enter discard
    // during the later ready turn, so an attack by unevolved Regidrago V cannot advance
    // that axis for them:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
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

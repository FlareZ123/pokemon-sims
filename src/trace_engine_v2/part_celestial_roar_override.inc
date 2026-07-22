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
    const bool held_gladion_prized_treasure_route =
        deck_seen_ && prizes_known() && !item_locked() &&
        hand_count(Card::Gladion) > 0 && held_dragon_payload &&
        prize_count_after_reveal(Card::MysteriousTreasure) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
    const bool held_supporter_payload_route =
        (hand_count(Card::Serena) > 0 && held_dragon_payload) ||
        (deck_seen_ && hand_count(Card::ProfessorBurnet) > 0 &&
         std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) ||
        held_gladion_prized_treasure_route;
    const bool held_energy_and_payload_route_guarantee_next_window =
        strict_payload_timing() && state_.manual_energy_used &&
        state_.turn < scenario_.max_turn && missing_grass + missing_fire == 1 &&
        ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||
         (missing_fire == 1 && hand_count(Card::Fire) > 0)) &&
        held_supporter_payload_route;
    if (held_energy_and_payload_route_guarantee_next_window) {
      // The unevolved Regidrago V cannot become Regidrago VSTAR before a later legal
      // evolution window. One held Basic Energy already guarantees GGF through that
      // turn's manual attachment. Held Serena plus a Dragon, held Professor Burnet
      // plus a known deck Dragon, or held Gladion plus a known prized Mysterious
      // Treasure, a held Dragon cost, and a known deck Regidrago VSTAR guarantees a
      // legal strict-JIT payload and evolution route. Celestial Roar cannot improve the
      // earliest ready window and can randomly remove the unresolved VSTAR or connector:
      // Regidrago V and Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Regidrago VSTAR evolution and Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Manual attachment, evolution, attack, Item, Prize exchange, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Earliest complete route and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Existing confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1079
      // Confirmed Gladion-Treasure continuation bug: https://github.com/FlareZ123/pokemon-sims/issues/1174
      trace("HOLD ATTACK", "R-RV-01; R-GAME-ENERGY; R-SERENA-01; R-BURNET-01; R-GLADION-01; R-MT-01; P-JIT-01; P-COMPRESS-01",
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

  bool retreat_to_benched_regidrago_v_for_celestial_roar_with_latias() {
    // Keep no-discard-control on its established deterministic continuation policy.
    // This promotion fixes the JIT route where an attack now is needed to preserve
    // the earliest setup window; banking profiles may already have their payload:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/1325
    if (!strict_payload_timing() || !can_free_retreat_with_latias()) return false;

    std::optional<std::size_t> best_index;
    int best_score = -1;
    for (std::size_t index = 0; index < state_.bench.size(); ++index) {
      const Pokemon& candidate = state_.bench[index];
      if (candidate.card != Card::RegidragoV) continue;

      // Reuse the exact Celestial Roar policy on a silent state projection. This
      // keeps every K0/K1, DCI, Energy, payload, and attack-hold gate identical
      // before Skyliner spends the once-per-turn Retreat action:
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Core Retreat and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest legal ready-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1325
      Engine projected = *this;
      projected.trace_ = nullptr;
      std::swap(*projected.state_.active, projected.state_.bench[index]);
      if (!projected.use_celestial_roar()) continue;

      const int score = 100 * std::min(2, candidate.grass) +
                        100 * std::min(1, candidate.fire);
      if (!best_index.has_value() || score > best_score) {
        best_index = index;
        best_score = score;
      }
    }
    if (!best_index.has_value()) return false;

    std::swap(*state_.active, state_.bench[*best_index]);
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-RV-01; R-GAME-RETREAT",
          "Latias ex gives the Basic Active no Retreat Cost; promoted an energized "
          "Benched Regidrago V for the live Celestial Roar route.");
    return true;
  }


  bool use_celestial_roar_after_latias_promotion() {
    // The final shell attack phase may first use Skyliner to promote an energized
    // Benched Regidrago V. Retry Powerglass after that Retreat so the Tool attaches
    // to the actual Active holder before Celestial Roar resolves:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1325
    retreat_to_benched_regidrago_v_for_celestial_roar_with_latias();
    attach_powerglass();
    return use_celestial_roar();
  }

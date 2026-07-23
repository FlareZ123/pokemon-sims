      const auto issue_1447_should_hold_vessel = [this] {
        if (scenario_.dci != DciProfile::StrictJit ||
            scenario_.locks != LockMode::None || state_.turn != 2 ||
            scenario_.max_turn < 3 || !deck_seen_ || !state_.active ||
            state_.active->card != Card::RegidragoVstar ||
            hand_count(Card::EarthenVessel) == 0 ||
            !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
          return false;
        }

        // Preserve Earthen Vessel only when the public K1 route proves that Crispin
        // plus the unused T2 manual attachment creates GF, then the held Dragon can
        // pay Vessel's T3 cost while its searched Grass completes GGF. This keeps the
        // same earliest ready turn and preserves Brilliant Blender:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
        // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Supporter, Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1447
        const bool before_crispin = !state_.supporter_used &&
            !state_.manual_energy_used && hand_count(Card::Crispin) > 0 &&
            state_.active->grass == 0 && state_.active->fire == 0 &&
            deck_count_after_search_started(Card::Grass) >= 2 &&
            deck_count_after_search_started(Card::Fire) >= 1;
        const bool after_crispin_before_manual = state_.supporter_used &&
            !state_.manual_energy_used && state_.active->grass == 1 &&
            state_.active->fire == 0 && hand_count(Card::Fire) > 0 &&
            deck_count_after_search_started(Card::Grass) >= 1;
        const bool after_crispin_and_manual = state_.supporter_used &&
            state_.manual_energy_used && state_.active->grass == 1 &&
            state_.active->fire == 1 &&
            deck_count_after_search_started(Card::Grass) >= 1;
        return before_crispin || after_crispin_before_manual ||
            after_crispin_and_manual;
      };

      const auto issue_1447_should_play_vessel_for_ready_turn = [this] {
        // On T3, the held Vessel itself is the legal same-turn payload outlet. Admit
        // its Dragon cost before the generic early Item pass disables payload costs;
        // the searched Grass remains in hand for the unused manual attachment:
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Item, discard, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT definition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1447
        return scenario_.dci == DciProfile::StrictJit &&
            scenario_.locks == LockMode::None && state_.turn == 3 &&
            deck_seen_ && state_.active &&
            state_.active->card == Card::RegidragoVstar &&
            state_.active->grass == 1 && state_.active->fire == 1 &&
            !state_.manual_energy_used && need_energy() && need_payload() &&
            hand_count(Card::EarthenVessel) > 0 &&
            deck_count_after_search_started(Card::Grass) > 0 &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
      };

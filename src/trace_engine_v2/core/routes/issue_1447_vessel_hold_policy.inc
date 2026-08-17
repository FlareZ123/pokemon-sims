      const auto issue_1447_should_hold_vessel = [this] {
        // The accepted #1447 comparison is current turn versus the immediately
        // following turn. The T2->T3 witness was historical seed identity only:
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
        // Repository route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed relative-turn bug: https://github.com/FlareZ123/pokemon-sims/issues/3561
        const int ready_turn = state_.turn + 1;
        if (state_.issue_1447_vessel_ready_turn == state_.turn || // A recorded next-turn continuation resolves now; never re-bank it: https://github.com/FlareZ123/pokemon-sims/issues/3561
            secret_box_combo_enabled() ||
            scenario_.dci != DciProfile::StrictJit ||
            scenario_.locks != LockMode::None ||
            scenario_.max_turn < ready_turn || !deck_seen_ || !state_.active ||
            state_.active->card != Card::RegidragoVstar ||
            hand_count(Card::EarthenVessel) == 0 ||
            !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
          return false;
        }

        // Once Crispin or another legal attachment already makes the Active
        // Regidrago VSTAR pay Apex, holding Vessel would delay a legal same-turn
        // Dragon-payload discard. DDE + Grass and DDE + Fire are both complete:
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
        if (pays_apex_energy_cost(*state_.active)) return false;

        // Preserve Earthen Vessel only when the public K1 shell route proves that
        // Crispin plus the unused manual attachment creates GF, then a held or
        // newly drawn Dragon can pay Vessel's next-turn cost while its searched
        // Grass completes GGF. Pineco's stored direct-Treasure continuation remains
        // owned by its dedicated planner and must not enter this shell-only hold:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
        // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
        // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Supporter, Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Pineco direct route: https://github.com/FlareZ123/pokemon-sims/issues/1420
        // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1447
        // Relative-turn correction: https://github.com/FlareZ123/pokemon-sims/issues/3561
        const bool before_crispin = !state_.supporter_used &&
            !state_.manual_energy_used && hand_count(Card::Crispin) > 0 &&
            state_.active->grass == 0 && state_.active->fire == 0 &&
            state_.active->double_dragon == 0 && // https://github.com/FlareZ123/pokemon-sims/issues/2368
            deck_count_after_search_started(Card::Grass) >= 2 &&
            deck_count_after_search_started(Card::Fire) >= 1;
        const bool after_crispin_before_manual = state_.supporter_used &&
            !state_.manual_energy_used && state_.active->grass == 1 &&
            state_.active->fire == 0 && hand_count(Card::Fire) > 0 &&
            deck_count_after_search_started(Card::Grass) >= 1;
        // A Powerglass attachment cannot satisfy this current turn's Apex Dragon
        // requirement because Powerglass resolves only after the attack window.
        // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Attack/end-of-turn sequence: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
        // Ready-state boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#ready-state-observation-boundary
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4227
        const bool after_crispin_and_manual = state_.supporter_used &&
            state_.manual_energy_used && state_.active->grass == 1 &&
            state_.active->fire == 1 &&
            deck_count_after_search_started(Card::Grass) >= 1;
        const bool should_hold = before_crispin || after_crispin_before_manual ||
            after_crispin_and_manual;
        if (should_hold) {
          state_.issue_1447_vessel_ready_turn = ready_turn; // Next-turn route state: https://github.com/FlareZ123/pokemon-sims/issues/3561
        }
        return should_hold;
      };

      const auto issue_1447_should_play_vessel_for_ready_turn = [this] {
        if (state_.issue_1447_vessel_ready_turn.has_value() &&
            state_.turn > *state_.issue_1447_vessel_ready_turn) {
          state_.issue_1447_vessel_ready_turn.reset(); // Expire missed continuation: https://github.com/FlareZ123/pokemon-sims/issues/3561
        }
        const auto completing_basic = state_.active
            ? issue_2437_apex_completing_basic_from_known_deck(*state_.active)
            : std::optional<Card>{};
        // On the recorded immediately-following turn, the held Vessel itself is the
        // legal same-turn payload outlet. Its searched Basic must be the one-card
        // semantic Apex completion, including DDE-only states where Grass and Fire
        // are alternative legal targets:
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Item, discard, search, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT definition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
        // Pineco direct route: https://github.com/FlareZ123/pokemon-sims/issues/1420
        // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1447
        // Relative-turn correction: https://github.com/FlareZ123/pokemon-sims/issues/3561
        return !secret_box_combo_enabled() &&
            scenario_.dci == DciProfile::StrictJit &&
            scenario_.locks == LockMode::None &&
            state_.issue_1447_vessel_ready_turn == state_.turn && // Recorded next turn only: https://github.com/FlareZ123/pokemon-sims/issues/3561
            deck_seen_ && state_.active &&
            state_.active->card == Card::RegidragoVstar &&
            !state_.manual_energy_used && need_energy() && need_payload() &&
            hand_count(Card::EarthenVessel) > 0 &&
            completing_basic.has_value() &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
      };

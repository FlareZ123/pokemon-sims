      const auto issue_1447_should_hold_vessel = [this] {
        if (secret_box_combo_enabled() ||
            scenario_.dci != DciProfile::StrictJit ||
            scenario_.locks != LockMode::None || state_.turn != 2 ||
            scenario_.max_turn < 3 || !deck_seen_ || !state_.active ||
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
        // Crispin plus the unused T2 manual attachment creates GF, then a held or
        // newly drawn Dragon can pay Vessel's T3 cost while its searched Grass
        // completes GGF. Pineco's stored direct-Treasure continuation remains owned
        // by its dedicated planner and must not enter this shell-only hold:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
        // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
        // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Supporter, Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Pineco direct route: https://github.com/FlareZ123/pokemon-sims/issues/1420
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1447
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
        const bool after_crispin_and_manual = state_.supporter_used &&
            state_.manual_energy_used && state_.active->grass == 1 &&
            state_.active->fire == 1 &&
            deck_count_after_search_started(Card::Grass) >= 1;
        return before_crispin || after_crispin_before_manual ||
            after_crispin_and_manual;
      };

      const auto issue_1447_should_play_vessel_for_ready_turn = [this] {
        const auto completing_basic = state_.active
            ? issue_2437_apex_completing_basic_from_known_deck(*state_.active)
            : std::optional<Card>{};
        // On T3, the held Vessel itself is the legal same-turn payload outlet. Its
        // searched Basic must be the one-card semantic Apex completion, including
        // DDE-only states where Grass and Fire are alternative legal targets:
        // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Item, discard, search, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Strict-JIT definition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
        // Pineco direct route: https://github.com/FlareZ123/pokemon-sims/issues/1420
        // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1447
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
        return !secret_box_combo_enabled() &&
            scenario_.dci == DciProfile::StrictJit &&
            scenario_.locks == LockMode::None && state_.turn == 3 &&
            deck_seen_ && state_.active &&
            state_.active->card == Card::RegidragoVstar &&
            !state_.manual_energy_used && need_energy() && need_payload() &&
            hand_count(Card::EarthenVessel) > 0 &&
            completing_basic.has_value() &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
      };

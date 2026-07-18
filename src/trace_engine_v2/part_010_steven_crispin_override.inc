  bool steven_should_yield_to_crispin_mysterious_route() const {
    const bool exact_lock_window = scenario_.locks == LockMode::TurnTwoItem &&
        state_.turn == 1 && !scenario_.going_first && !item_locked();
    const bool active_regi_can_attack = state_.active &&
        state_.active->card == Card::RegidragoV && !state_.manual_energy_used;
    const bool inspected_vstar_and_crispin_targets = deck_seen_ &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0;
    const bool payable_mysterious_route = hand_count(Card::MysteriousTreasure) > 0 &&
        is_legal_cost_available(false, true);

    // Wonder Tag has already established K1. Suppress Steven only when the current
    // public state proves a payable Mysterious Treasure route to Regidrago VSTAR and
    // Crispin can power the Active Regidrago V while the manual attachment remains.
    // Steven would end the turn and delay the repository's earliest-ready objective:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/774
    return exact_lock_window && active_regi_can_attack &&
        inspected_vstar_and_crispin_targets && payable_mysterious_route &&
        crispin_can_advance_energy_axis();
  }

  bool wonder_tag_can_bank_steven_for_known_t3_blender_route() const {
    if (state_.turn != 1 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !deck_seen_ || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::RegidragoVstar) > 0 || hand_count(Card::Crispin) > 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::Fire) == 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::StevensResolve) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    const bool fresh_benched_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn <= state_.turn &&
              pokemon.grass == 0 && pokemon.fire == 0;
        });

    // Going first prevents the turn-one Supporter play, while Wonder Tag can still
    // bank Steven's Resolve. The public Grass and Fire cover the first two manual
    // attachments. Steven then takes Regidrago VSTAR, Crispin, and Latias ex; on T3,
    // Crispin supplies the final GGF component, held Brilliant Blender supplies the
    // same-turn payload, and Skyliner promotes the Benched attacker:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/946
    return fresh_benched_regidrago;
  }

  bool banked_steven_has_known_t3_blender_route() const {
    if (state_.turn != 2 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || !need_payload() ||
        !payload_might_be_in_deck() ||
        (hand_count(Card::RegidragoVstar) == 0 &&
         deck_count_after_search_started(Card::RegidragoVstar) == 0) ||
        (hand_count(Card::Crispin) == 0 &&
         deck_count_after_search_started(Card::Crispin) == 0) ||
        (hand_count(Card::LatiasEx) == 0 &&
         deck_count_after_search_started(Card::LatiasEx) == 0) ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      int projected_grass = pokemon.grass;
      int projected_fire = pokemon.fire;
      if (!state_.manual_energy_used) {
        if (projected_grass < 2 && hand_count(Card::Grass) > 0) {
          ++projected_grass;
        } else if (projected_fire < 1 && hand_count(Card::Fire) > 0) {
          ++projected_fire;
        } else {
          continue;
        }
      }

      if (std::max(0, 2 - projected_grass) +
              std::max(0, 1 - projected_fire) ==
          1) {
        // The turn-two attachment leaves one GGF component. Steven's unrestricted
        // three-card search reserves VSTAR, Crispin, and Latias; Crispin attaches the
        // final component on T3 before Blender and Skyliner complete strict readiness:
        // https://api.pokemontcg.io/v2/cards/sm7-145
        // https://api.pokemontcg.io/v2/cards/sv7-133
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/sv8-164
        // https://api.pokemontcg.io/v2/cards/sv8-76
        // https://www.pokemon.com/us/pokemon-tcg/rules
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // https://github.com/FlareZ123/pokemon-sims/issues/946
        return true;
      }
    }
    return false;
  }

  Card choose_supporter_after_search_started() {
    if (wonder_tag_can_bank_steven_for_known_t3_blender_route()) {
      return Card::StevensResolve;
    }
    return static_cast<const Engine&>(*this).choose_supporter_after_search_started();
  }

  bool should_play_steven() const {
    return (should_play_steven_original() &&
            !steven_should_yield_to_crispin_mysterious_route()) ||
        late_steven_is_only_basic_connector() ||
        late_steven_has_known_t3_compression_route() ||
        banked_steven_has_known_t3_blender_route();
  }

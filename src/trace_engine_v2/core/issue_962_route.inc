#if defined(REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY)
      const auto quick_ball_tapu_crispin_route_should_precede_vessel =
          [this, permit_payload]() {
        if (!need_energy() || item_locked() || hand_count(Card::QuickBall) == 0 ||
            hand_count(Card::EarthenVessel) == 0 || hand_count(Card::TapuLeleGX) > 0 ||
            in_play(Card::TapuLeleGX) || bench_space() <= 0 || !supporter_allowed() ||
            !ability_available_for_pokemon(Card::TapuLeleGX) ||
            hand_count(Card::Crispin) > 0 || !crispin_can_advance_energy_axis() ||
            !might_be_unseen(Card::TapuLeleGX) || !might_be_unseen(Card::Crispin) ||
            !needs_tapu_connector()) {
          return false;
        }

        // Vessel plus the unused manual attachment already completes a one-Energy
        // deficit, so the extra Basic, Bench slot, and Supporter connector add no setup
        // speed in that state: https://api.pokemontcg.io/v2/cards/sv4-163
        // https://www.pokemon.com/us/pokemon-tcg/rules
        const Pokemon* vessel_target = target_regi();
        const bool vessel_and_manual_complete = !state_.manual_energy_used &&
            vessel_target != nullptr &&
            completing_basic_energy_from_possible_deck(*vessel_target).has_value();
        // DDE provides two units of every Energy type, so typed deficits are
        // alternatives and must not be added as physical-card distance:
        // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://github.com/FlareZ123/pokemon-sims/issues/2368
        if (vessel_and_manual_complete) return false;

        const auto pay_quick_ball_cost = [permit_payload](Engine& projected) {
          const bool can_jit_discard = permit_payload &&
              projected.can_play_payload_this_turn() &&
              projected.blender_vstar_card_axis_can_finish_this_turn() &&
              std::any_of(projected.state_.hand.begin(), projected.state_.hand.end(),
                          is_payload);
          const auto cost = projected.choose_discard(
              can_jit_discard, true, true, Card::QuickBall);
          if (!cost || !remove_one(projected.state_.hand, Card::QuickBall)) {
            return false;
          }
          return projected.discard_from_hand(
              *cost, "projected Quick Ball cost", "R-QB-01");
        };
#elif defined(REGIDRAGO_ISSUE_962_SECTION_PROJECTION)
      const auto pay_vessel_cost = [permit_payload](Engine& projected) {
        if (projected.hold_payload_outlet_for_post_powerglass_turn()) return false;
        const bool payload_cost_permitted = permit_payload &&
            (projected.payload_ready() ||
             projected.blender_vstar_card_axis_can_finish_this_turn());
        const bool can_jit_discard = payload_cost_permitted &&
            (projected.can_play_payload_this_turn() || projected.payload_ready()) &&
            std::any_of(projected.state_.hand.begin(), projected.state_.hand.end(),
                        is_payload);
        const bool needed_energy_might_be_in_deck =
            (projected.grass_needed() > 0 && projected.might_be_unseen(Card::Grass)) ||
            (projected.fire_needed() > 0 && projected.might_be_unseen(Card::Fire));
        if (!needed_energy_might_be_in_deck && !can_jit_discard) return false;
        const auto cost = projected.choose_discard(
            can_jit_discard, true, true, Card::EarthenVessel);
        if (!cost || !remove_one(projected.state_.hand, Card::EarthenVessel)) {
          return false;
        }
        return projected.discard_from_hand(
            *cost, "projected Earthen Vessel cost", "R-EV-01");
      };

      const auto project_vessel_search = [](Engine& projected) {
        int remaining = 2;
        const auto take = [&projected, &remaining](const Card energy) {
          if (remaining == 0 || !projected.might_be_unseen(energy)) return;
          if (projected.deck_seen_ && !remove_one(projected.state_.deck, energy)) {
            return;
          }
          projected.state_.hand.push_back(energy);
          --remaining;
        };
        if (projected.grass_needed() > 0) take(Card::Grass);
        if (projected.fire_needed() > 0) take(Card::Fire);
        while (remaining > 0) {
          const std::size_t before = projected.state_.hand.size();
          if (projected.grass_needed() > 0) take(Card::Grass);
          if (remaining > 0 && projected.fire_needed() > 0) take(Card::Fire);
          if (projected.state_.hand.size() == before) break;
        }
      };

      const auto make_shadow = [this](std::mt19937_64& shadow_rng) {
        Engine projected(scenario_, recipe_, shadow_rng);
        projected.state_ = state_;
        projected.deck_seen_ = deck_seen_;
        projected.prizes_revealed_ = prizes_revealed_;
        return projected;
      };
#elif defined(REGIDRAGO_ISSUE_962_SECTION_DECISION)
      std::mt19937_64 quick_rng{962};
      Engine quick_first = make_shadow(quick_rng);
      if (!pay_quick_ball_cost(quick_first)) return false;
      quick_first.state_.bench.push_back(
          Pokemon{Card::TapuLeleGX, quick_first.state_.turn});
      quick_first.state_.hand.push_back(Card::Crispin);
      if (!pay_vessel_cost(quick_first)) return false;
      project_vessel_search(quick_first);
      const bool quick_first_route_live =
          quick_first.crispin_can_advance_energy_axis();
      if (!quick_first_route_live) return false;

      std::mt19937_64 vessel_rng{963};
      Engine vessel_first = make_shadow(vessel_rng);
      if (!pay_vessel_cost(vessel_first)) return false;
      project_vessel_search(vessel_first);
      const bool vessel_first_route_live = vessel_first.needs_tapu_connector() &&
          pay_quick_ball_cost(vessel_first);

      // Quick Ball discards before searching a Basic Pokémon, Tapu Lele-GX must be
      // Benched before Wonder Tag searches Crispin, Earthen Vessel pays a separate
      // discard before its Basic Energy search, and Crispin uses the one Supporter
      // play to attach one searched Energy and put the other into hand. Preempt
      // Vessel only when Quick Ball-first preserves both paid effects while
      // Vessel-first exhausts the legal Quick Ball cost:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sm2-60
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // https://github.com/FlareZ123/pokemon-sims/issues/962
      return !vessel_first_route_live;
    };
#else
#error "Select exactly one issue-962 route section before including this file"
#endif

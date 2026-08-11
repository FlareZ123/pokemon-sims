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

          const Pokemon* vessel_target = target_regi();
          const bool vessel_and_manual_complete = !state_.manual_energy_used &&
              vessel_target != nullptr &&
              issue_2437_apex_completing_basic_from_possible_deck(
                  *vessel_target).has_value();
          // Vessel plus the unused manual attachment already completes the Energy
          // axis when one searchable Basic semantically pays Apex after attachment.
          // DDE makes Grass and Fire alternative one-card completions, so spending
          // Quick Ball, a discard, a Bench slot, Tapu, and the Supporter is slower:
          // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
          // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Official Item, Ability, Supporter, search, discard, Bench, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Connector-cost and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
          // Original ordering bug: https://github.com/FlareZ123/pokemon-sims/issues/962
          // Confirmed DDE bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
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

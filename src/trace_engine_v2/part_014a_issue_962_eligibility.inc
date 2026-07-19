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
          const bool vessel_and_manual_complete = !state_.manual_energy_used &&
              grass_needed() + fire_needed() == 1 &&
              ((grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
               (fire_needed() == 1 && might_be_unseen(Card::Fire)));
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

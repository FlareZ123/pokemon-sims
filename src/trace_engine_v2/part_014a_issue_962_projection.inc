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

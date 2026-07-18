  bool has_live_ultra_ball_hand_payload_line() const {
    if (item_locked() || hand_count(Card::UltraBall) == 0 ||
        !can_play_payload_this_turn() || !ultra_ball_has_legal_target()) {
      return false;
    }

    // Ultra Ball may discard the held Dragon payload plus one other policy-legal
    // card, then search a Pokemon. Simulate the exact hand while reserving the
    // Ultra Ball being played instead of treating its presence as sufficient:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    Engine simulated = *this;
    const auto payload_cost = simulated.choose_discard(true, true, true, Card::UltraBall);
    if (!payload_cost || !is_payload(*payload_cost) ||
        !remove_one(simulated.state_.hand, *payload_cost)) {
      return false;
    }
    return simulated.choose_discard(false, true, true, Card::UltraBall).has_value();
  }

  bool oricorio_preserves_burnet_with_current_hand() const {
    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_might_be_in_deck =
        (grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
        (fire_needed() == 1 && might_be_unseen(Card::Fire));
    const bool payload_in_hand =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool live_hand_payload_outlet = payload_in_hand &&
        (has_live_one_discard_hand_payload_line() ||
         has_live_ultra_ball_hand_payload_line());

    // An in-hand Dragon does not complete strict-JIT readiness by itself. When no
    // legal Item can discard it this turn, Vital Dance must keep the Supporter play
    // available for Professor Burnet's deck-to-discard payload bridge. A live Item
    // outlet keeps the existing Tapu Lele-GX into Crispin route available:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return supporter_allowed() && !state_.manual_energy_used &&
           exactly_one_energy_missing && needed_energy_might_be_in_deck &&
           can_play_payload_this_turn() && hand_count(Card::ProfessorBurnet) > 0 &&
           !has_live_blender_payload_line() && payload_might_be_in_deck() &&
           !live_hand_payload_outlet;
  }

  bool original_wonder_tag_route_has_live_target() {
    // Wonder Tag searches a Supporter. At K1, the Supporter that justifies spending
    // Tapu Lele-GX and a Bench slot must actually remain in the inspected deck;
    // K0 keeps fixed-list uncertainty through might_be_unseen():
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool live_t1_steven = state_.turn == 1 && !scenario_.going_first &&
        should_play_steven() && hand_count(Card::StevensResolve) == 0 &&
        might_be_unseen(Card::StevensResolve);
    // Steven's Resolve searches up to 3 cards and ends the turn:
    // https://api.pokemontcg.io/v2/cards/sm7-145

    const bool live_direct_crispin = need_energy() &&
        hand_count(Card::Crispin) == 0 && crispin_can_advance_energy_axis() &&
        might_be_unseen(Card::Crispin);
    // Crispin advances the Energy axis only while a needed Basic Energy remains:
    // https://api.pokemontcg.io/v2/cards/sv7-133

    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_might_be_in_deck =
        (grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
        (fire_needed() == 1 && might_be_unseen(Card::Fire));
    const bool live_arven_vessel_energy = need_energy() && exactly_one_energy_missing &&
        !state_.manual_energy_used && !item_locked() && needed_energy_might_be_in_deck &&
        hand_count(Card::Arven) == 0 && might_be_unseen(Card::Arven) &&
        might_be_unseen(Card::EarthenVessel) &&
        is_legal_cost_available(can_play_payload_this_turn(), true);
    // Arven may find Earthen Vessel, whose paid search can put the final Basic
    // Energy into hand for the unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules

    // Burnet is an indirect payload bridge. Do not spend Tapu Lele-GX, a Bench
    // slot, and Wonder Tag when a held Supporter or target-legal discard Item can
    // already discard the permitted Dragon for the same current-turn payload axis:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/888
    // https://github.com/FlareZ123/pokemon-sims/issues/942
    const bool live_hand_payload_item = has_live_one_discard_hand_payload_line() ||
        has_live_ultra_ball_hand_payload_line();
    const bool live_burnet = can_play_payload_this_turn() &&
        payload_might_be_in_deck() && hand_count(Card::ProfessorBurnet) == 0 &&
        !has_live_blender_payload_line() && !has_live_serena_hand_payload_line() &&
        !live_hand_payload_item && might_be_unseen(Card::ProfessorBurnet);
    // Professor Burnet searches and discards up to 2 deck cards:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26

    const bool direct_ultra_vstar_route = payable_ultra_ball_vstar_route();
    const bool live_arven_vstar = need_vstar() && hand_count(Card::Arven) == 0 &&
        hand_count(Card::EvolutionIncense) == 0 &&
        hand_count(Card::MysteriousTreasure) == 0 && !direct_ultra_vstar_route &&
        might_be_unseen(Card::Arven);
    // Arven is the Supporter target for the modeled Item-to-VSTAR route:
    // https://api.pokemontcg.io/v2/cards/sv1-166

    const bool live_gladion_axis = prizes_known() && might_be_unseen(Card::Gladion) &&
        (final_prized_energy_for_gladion().has_value() ||
         (need_vstar() && !might_be_unseen(Card::RegidragoVstar) &&
          known_prize(Card::RegidragoVstar)));
    // Gladion remains a live Wonder Tag target when K1 proves the exact missing
    // VSTAR or final attachable Energy is in the Prize cards:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

    // The active Tapu preflight wraps the original selector. Keep the wrapper's
    // target-validation gate aligned with the new K1 Arven held-payload route, or
    // the original selector's true result is discarded before Wonder Tag resolves:
    // Wonder Tag: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/648
    const bool live_arven_held_payload =
        wonder_tag_arven_held_payload_route_available();
    const bool live_arven_communication = !item_locked() &&
        hand_count(Card::Arven) == 0 && might_be_unseen(Card::Arven) &&
        might_be_unseen(Card::PokemonCommunication) &&
        pokemon_communication_plan_after_wonder_tag_arven().has_value();
    // Wonder Tag may find Arven, which may search Pokémon Communication, only when
    // the exact post-Supporter exchange is live:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/issues/701

    return live_t1_steven || live_direct_crispin || live_arven_vessel_energy ||
           live_burnet || live_arven_vstar || live_gladion_axis ||
           live_arven_held_payload || live_arven_communication;
  }

  bool future_turn_wonder_tag_route_has_live_target() {
    if (state_.turn != 1 || !scenario_.going_first || bench_space() <= 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX)) {
      return false;
    }

    // Preserve the last Bench slot for Vital Dance when Oricorio can advance the
    // Energy axis immediately. Banking a Supporter is future value and must not
    // displace a stronger legal current-turn connector:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (bench_space() == 1 && hand_count(Card::Oricorio) > 0 &&
        !in_play(Card::Oricorio) && needs_oricorio_connector()) {
      return false;
    }

    const bool live_gladion_axis = prizes_known() &&
        hand_count(Card::Gladion) == 0 && might_be_unseen(Card::Gladion) &&
        (final_prized_energy_for_gladion().has_value() ||
         (need_vstar() && !might_be_unseen(Card::RegidragoVstar) &&
          known_prize(Card::RegidragoVstar)));

    const bool holder_for_fss =
        (state_.active && state_.active->card == Card::RegidragoV &&
         state_.active->tool == Tool::None) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV && pokemon.tool == Tool::None;
        });
    const bool direct_vstar_connector = hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::EvolutionIncense) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 || payable_ultra_ball_vstar_route() ||
        (hand_count(Card::ForestSealStone) > 0 && !state_.vstar_power_used &&
         holder_for_fss);
    const bool arven_has_concrete_vstar_chain =
        might_be_unseen(Card::RegidragoVstar) &&
        (might_be_unseen(Card::EvolutionIncense) ||
         (!state_.vstar_power_used && holder_for_fss &&
          might_be_unseen(Card::ForestSealStone)));
    const bool live_arven_vstar = need_vstar() && hand_count(Card::Arven) == 0 &&
        might_be_unseen(Card::Arven) && !direct_vstar_connector &&
        arven_has_concrete_vstar_chain;

    const auto future_crispin_completes_ggf = [&]() {
      if (!need_energy() || state_.manual_energy_used ||
          hand_count(Card::Crispin) > 0 || !might_be_unseen(Card::Crispin) ||
          !might_be_unseen(Card::Grass) || !might_be_unseen(Card::Fire)) {
        return false;
      }

      const int initial_grass_needed = grass_needed();
      const int initial_fire_needed = fire_needed();
      const int held_grass = hand_count(Card::Grass);
      const int held_fire = hand_count(Card::Fire);

      const auto completes_after_current_manual = [&](const int current_manual) {
        int remaining_grass = initial_grass_needed;
        int remaining_fire = initial_fire_needed;
        int grass_in_hand = held_grass;
        int fire_in_hand = held_fire;

        if (current_manual == 1) {
          if (remaining_grass <= 0 || grass_in_hand <= 0) return false;
          --remaining_grass;
          --grass_in_hand;
        } else if (current_manual == 2) {
          if (remaining_fire <= 0 || fire_in_hand <= 0) return false;
          --remaining_fire;
          --fire_in_hand;
        }

        const auto completes_after_crispin = [&](const bool attach_grass) {
          int grass_after_crispin = remaining_grass;
          int fire_after_crispin = remaining_fire;
          int grass_available_for_manual = grass_in_hand;
          int fire_available_for_manual = fire_in_hand;

          if (attach_grass) {
            if (grass_after_crispin <= 0) return false;
            --grass_after_crispin;
            ++fire_available_for_manual;
          } else {
            if (fire_after_crispin <= 0) return false;
            --fire_after_crispin;
            ++grass_available_for_manual;
          }

          if (grass_after_crispin == 0 && fire_after_crispin == 0) return true;
          if (grass_after_crispin == 1 && fire_after_crispin == 0) {
            return grass_available_for_manual > 0;
          }
          if (grass_after_crispin == 0 && fire_after_crispin == 1) {
            return fire_available_for_manual > 0;
          }
          return false;
        };

        return completes_after_crispin(true) || completes_after_crispin(false);
      };

      // Wonder Tag may bank Crispin on the first player's opening turn. Enumerate
      // the unused current manual attachment, Crispin's different-type search and
      // attachment, and the next turn's manual attachment. This admits the route
      // only when those public resources complete GGF without Vital Dance or Star
      // Alchemy:
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/809
      return completes_after_current_manual(0) ||
             completes_after_current_manual(1) ||
             completes_after_current_manual(2);
    };
    const bool live_crispin_energy = future_crispin_completes_ggf();

    // Going first prohibits playing a Supporter on turn one. Wonder Tag may still
    // search a concrete Gladion, Arven, or Crispin and leave it in hand for turn two:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/748
    // https://github.com/FlareZ123/pokemon-sims/issues/809
    return live_gladion_axis || live_arven_vstar || live_crispin_energy;
  }

  bool needs_tapu_connector() {
    // With one Bench slot left, suppress Wonder Tag into Crispin when an in-hand
    // payload is stranded and Oricorio preserves Professor Burnet's only complete
    // current-turn route. This extends the completed #180 selector to the usable-
    // payload distinction documented in #616:
    // https://github.com/FlareZ123/pokemon-sims/issues/180
    // https://github.com/FlareZ123/pokemon-sims/issues/616
    if (bench_space() == 1 && hand_count(Card::Oricorio) > 0 &&
        !in_play(Card::Oricorio) && oricorio_preserves_burnet_with_current_hand()) {
      return false;
    }

    if (needs_tapu_connector_original() && original_wonder_tag_route_has_live_target()) {
      return true;
    }
    if (future_turn_wonder_tag_route_has_live_target()) return true;

    // Wonder Tag can search Tate & Liza, whose switch mode immediately promotes a
    // Benched Regidrago VSTAR. Spend the Bench slot and Ability only when the actual
    // promotion target already pays Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // A player may use the still-open Supporter play before attacking:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // When the payload is still held, a payable one-discard Item may establish that
    // axis after Tate switches. Wonder Tag must leave a card in deck for the Item's
    // later required search, so the pre-search deck needs at least two cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_tate_blender_tate_override.inc#L1-L22
    const bool payload_axis_completable = !need_payload() ||
        (state_.deck.size() > 1U && has_live_one_discard_hand_payload_line());
    const bool direct_communication_latias_route = need_active_vstar() &&
        pokemon_communication_has_direct_target_route(Card::LatiasEx);
    const bool tate_completes_active_axis = need_active_vstar() &&
        benched_vstar_promotion_ready() && payload_axis_completable &&
        !can_free_retreat_with_latias() && !direct_communication_latias_route &&
        hand_count(Card::TateLiza) == 0 && might_be_unseen(Card::TateLiza);
    return supporter_allowed() && ability_available_for_pokemon(Card::TapuLeleGX) &&
           tate_completes_active_axis;
  }

  bool bench_tapu_if_useful() {
    if (hand_count(Card::TapuLeleGX) == 0 || in_play(Card::TapuLeleGX) ||
        bench_space() <= 0 || !needs_tapu_connector()) {
      return false;
    }
    return bench_from_hand(Card::TapuLeleGX, true);
  }

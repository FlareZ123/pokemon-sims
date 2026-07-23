  bool use_fss() {
    const auto live_fss_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    const bool forest_seal_stone_is_live = !state_.vstar_power_used &&
        ((state_.active && live_fss_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder));
    // Forest Seal Stone grants Star Alchemy only to the Pokemon V it is attached to.
    // Prove that the public board contains a live holder before any FSS-specific hold
    // trace or target evaluation can report preservation of that VSTAR Power:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
    // https://github.com/FlareZ123/pokemon-sims/issues/727
    if (!forest_seal_stone_is_live) return false;

    const bool incomplete_tate_blender_route =
        !state_.vstar_power_used && supporter_allowed() && need_active_vstar() &&
        !need_energy() && !benched_vstar_promotion_ready() && need_payload() &&
        !item_locked() && hand_count(Card::BrilliantBlender) > 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck() &&
        !can_free_retreat_with_latias() && hand_count(Card::TateLiza) == 0 &&
        might_be_unseen(Card::TateLiza);

    if (incomplete_tate_blender_route) {
      // Star Alchemy is a one-per-game VSTAR Power. Tate & Liza can promote the
      // Benched VSTAR and Brilliant Blender can supply a same-turn payload, but the
      // selected attacker must already pay Apex Dragon's GGF before that search is
      // a complete route. Preserve the VSTAR Power while the target is incomplete:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sm7-148
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      trace("HOLD VSTAR POWER", "R-FSS-02; R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Star Alchemy because Tate would promote an Energy-incomplete Regidrago VSTAR.");
      return false;
    }

    const bool tate_held_payload_candidate = !state_.vstar_power_used &&
        supporter_allowed() && need_active_vstar() && need_payload() &&
        !can_free_retreat_with_latias() && hand_count(Card::TateLiza) == 0 &&
        might_be_unseen(Card::TateLiza) && has_live_one_discard_hand_payload_line();
    if (tate_held_payload_candidate && !benched_vstar_promotion_ready()) {
      // Tate plus a held one-discard Item is useful only when the promoted VSTAR
      // already pays Apex Dragon's GGF cost:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sm7-148
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      trace("HOLD VSTAR POWER", "R-FSS-02; R-TATE-01; R-RVS-01; P-DCI-01",
            "Retained Star Alchemy because Tate would promote an Energy-incomplete Regidrago VSTAR.");
      return false;
    }
    if (tate_held_payload_candidate && state_.deck.size() <= 1U) {
      // Star Alchemy's mandatory search would remove the final deck card. The held
      // Mysterious Treasure, Quick Ball, or Earthen Vessel could then no longer begin
      // its required search after discarding the Dragon payload:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
      trace("HOLD VSTAR POWER", "R-FSS-02; R-GAME-ITEM; P-DCI-01",
            "Retained Star Alchemy because searching Tate would empty the deck before the held payload Item.");
      return false;
    }

    const bool latias_solves_only_missing_axis =
        !state_.vstar_power_used && need_active_vstar() && !need_regi() &&
        !need_vstar() && !need_energy() && !need_payload() &&
        can_free_retreat_with_latias();

    if (latias_solves_only_missing_axis) {
      // Latias ex already gives the Basic Active no Retreat Cost, and retreating once
      // during the turn can promote the complete Benched Regidrago VSTAR. Preserve
      // the one-per-game Star Alchemy resource when that is the sole missing axis:
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      trace("HOLD VSTAR POWER", "R-LATIAS-01; R-GAME-RETREAT; R-FSS-02; R-VSTAR-01",
            "Latias ex already supplies the complete free-retreat route to the Benched Regidrago VSTAR.");
      return false;
    }

    if (fss_should_hold_for_observable_complete_route()) {
      // The K1 shadow reached the same current-turn ready state without adding any
      // searched card. Preserve the once-per-game VSTAR Power and every one-shot
      // connector already held by the deterministic complete route. Repeated policy
      // stabilization for an unchanged state must emit this decision only once:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/990
      // https://github.com/FlareZ123/pokemon-sims/issues/1010
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-990-fss-observable-complete-route", state_.turn,
            "HOLD VSTAR POWER", "R-FSS-02; R-VSTAR-01; P-KNOWLEDGE-01; P-DCI-01",
            "Retained Star Alchemy because observable held resources already complete every current-turn setup axis.");
      }
      return false;
    }

    if (deck_seen_) {
      // K1 must preserve the once-per-game VSTAR Power when the inspected deck has no
      // card or complete chain that advances a currently unresolved setup axis:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      const Card planned_target = fss_target_after_search_started();
      const bool target_present = deck_count_after_search_started(planned_target) > 0;
      const bool needed_energy_present =
          (grass_needed() > 0 && deck_count_after_search_started(Card::Grass) > 0) ||
          (fire_needed() > 0 && deck_count_after_search_started(Card::Fire) > 0);
      const bool payload_present = payload_might_be_in_deck();
      const bool one_discard_cost_available =
          is_legal_cost_available(can_play_payload_this_turn(), true);
      const bool held_payload_is_only_missing_axis = need_payload() && !need_regi() &&
          !need_vstar() && !need_energy() && !need_active_vstar() &&
          can_play_payload_this_turn() && !item_locked() &&
          std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
      // A held Dragon can pay the Item's discard cost and complete strict JIT only
      // when the Item retains a legal search target in the inspected deck:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Trainer resolution ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
      // Strict-JIT contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
      const auto held_payload_item_advances = [this, held_payload_is_only_missing_axis](const Card item) {
        if (!held_payload_is_only_missing_axis) return false;
        switch (item) {
          case Card::MysteriousTreasure:
            return std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
          case Card::QuickBall:
            return std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);
          case Card::EarthenVessel:
            return deck_count_after_search_started(Card::Grass) > 0 ||
                deck_count_after_search_started(Card::Fire) > 0;
          default:
            return false;
        }
      };
      const bool arven_communication_chain =
          deck_count_after_search_started(Card::PokemonCommunication) > 0 &&
          pokemon_communication_plan(true, false, true).has_value();
      const bool arven_has_live_chain = supporter_allowed() && !item_locked() &&
          (arven_communication_chain ||
           (need_regi() && bench_space() > 0 &&
            ((deck_count_after_search_started(Card::RegidragoV) > 0 &&
              (deck_count_after_search_started(Card::QuickBall) > 0 ||
               deck_count_after_search_started(Card::MysteriousTreasure) > 0) &&
              one_discard_cost_available) ||
             (prizes_known() && prize_count_after_reveal(Card::RegidragoV) > 0 &&
              deck_count_after_search_started(Card::HisuianHeavyBall) > 0))) ||
           (need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
            (deck_count_after_search_started(Card::EvolutionIncense) > 0 ||
             (deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
              one_discard_cost_available))) ||
           (need_energy() && needed_energy_present &&
            deck_count_after_search_started(Card::EarthenVessel) > 0 &&
            one_discard_cost_available) ||
           (need_active_vstar() && bench_space() > 0 &&
            ability_available_for_pokemon(Card::LatiasEx) &&
            deck_count_after_search_started(Card::LatiasEx) > 0 &&
            (deck_count_after_search_started(Card::QuickBall) > 0 ||
             deck_count_after_search_started(Card::MysteriousTreasure) > 0) &&
            one_discard_cost_available) ||
           (need_payload() && can_play_payload_this_turn() && payload_present &&
            deck_count_after_search_started(Card::BrilliantBlender) > 0));
      const bool target_advances_axis = [&]() {
        switch (planned_target) {
          case Card::StevensResolve:
            return should_play_steven();
          case Card::RegidragoV:
            return need_regi() && bench_space() > 0;
          case Card::RegidragoVstar:
            return need_vstar();
          case Card::Gladion:
            return prizes_known() &&
                ((need_regi() && prize_count_after_reveal(Card::RegidragoV) > 0) ||
                 (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
                 final_prized_energy_for_gladion().has_value());
          case Card::LatiasEx:
            // The issue-1068 route proves Latias advances the complete promotion,
            // Energy, Evolution, and strict-JIT payload chain even though Skyliner is
            // unavailable until Latias enters play:
            // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
            // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
            // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1068
            return fss_latias_over_redundant_burnet_route_available() ||
                fss_latias_post_crispin_burnet_route_available() ||
                fss_latias_vessel_promotion_route_available() ||
                (need_active_vstar() && bench_space() > 0 &&
                 ability_available_for_pokemon(Card::LatiasEx));
          case Card::TateLiza:
            // The current-turn Tate route requires an available Supporter play.
            // Issue #1139 adds the separately proven next-turn banked route after
            // Arven has consumed that play and held Blender covers payload:
            // https://api.pokemontcg.io/v2/cards/sm7-148
            // https://api.pokemontcg.io/v2/cards/sv8-164
            // https://github.com/FlareZ123/pokemon-sims/issues/1139
            return (need_active_vstar() && supporter_allowed()) ||
                fss_should_bank_tate_for_next_turn_blender_route();
          case Card::BrilliantBlender:
            return need_payload() && can_play_payload_this_turn() && !item_locked() &&
                payload_present;
          case Card::ProfessorBurnet:
            // Arven can search Forest Seal Stone after consuming this turn's Supporter.
            // Star Alchemy may bank Professor Burnet for the next modeled turn, where
            // Burnet searches and discards the Dragon payload for Apex Dragon:
            // https://api.pokemontcg.io/v2/cards/sv1-166
            // https://api.pokemontcg.io/v2/cards/swsh12-156
            // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
            // https://api.pokemontcg.io/v2/cards/swsh12-136
            // https://github.com/FlareZ123/pokemon-sims/issues/705
            return need_payload() && can_play_payload_this_turn() && payload_present &&
                (supporter_allowed() || state_.turn < scenario_.max_turn);
          case Card::Crispin:
            return need_energy() && supporter_allowed() && crispin_can_advance_energy_axis();
          case Card::Oricorio:
            return need_energy() && bench_space() > 0 && needed_energy_present;
          case Card::EarthenVessel:
            return (need_energy() && !item_locked() && needed_energy_present &&
                    one_discard_cost_available) ||
                held_payload_item_advances(Card::EarthenVessel);
          case Card::Grass:
            return grass_needed() > 0 && !state_.manual_energy_used;
          case Card::Fire:
            return fire_needed() > 0 && !state_.manual_energy_used;
          case Card::Arven:
            return arven_has_live_chain;
          case Card::PokemonCommunication:
            return !item_locked() &&
                pokemon_communication_plan(true, false, false).has_value();
          case Card::MysteriousTreasure:
            return (!item_locked() && one_discard_cost_available &&
                    ((need_regi() && deck_count_after_search_started(Card::RegidragoV) > 0) ||
                     (need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) > 0) ||
                     (need_active_vstar() && deck_count_after_search_started(Card::LatiasEx) > 0))) ||
                held_payload_item_advances(Card::MysteriousTreasure);
          case Card::QuickBall:
            return (!item_locked() && one_discard_cost_available &&
                    ((need_regi() && deck_count_after_search_started(Card::RegidragoV) > 0) ||
                     (need_active_vstar() && deck_count_after_search_started(Card::LatiasEx) > 0))) ||
                held_payload_item_advances(Card::QuickBall);
          case Card::Serena:
            return need_payload() && can_play_payload_this_turn() && supporter_allowed() &&
                std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
          default:
            return false;
        }
      }();
      if (!target_present || !target_advances_axis) {
        // Star Alchemy remains unused, so repeated stabilization must not expose the
        // same K1 retention decision more than once for an equivalent state:
        // https://api.pokemontcg.io/v2/cards/swsh12-156
        // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
        // https://github.com/FlareZ123/pokemon-sims/issues/1010
        if (trace_ != nullptr) {
          trace_->add_policy_once(
              state_, "issue-1010-fss-k1-no-advancing-target", state_.turn,
              "HOLD VSTAR POWER", "R-FSS-02; R-VSTAR-01; P-KNOWLEDGE-01; P-DCI-01",
              "Retained Star Alchemy because K1 proved that no searched card advances setup.");
        }
        return false;
      }
    }

    return use_fss_latias_original();
  }

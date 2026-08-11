  bool blender_vstar_card_axis_can_finish_this_turn() const {
    if (issue_1646_hold_blender_for_burnet_finish_visible()) return false;
    if (!strict_payload_timing() || !need_vstar()) return true;
    if (hand_count(Card::RegidragoVstar) > 0) return true;

    // Brilliant Blender's one-use deck-to-discard effect advances strict JIT only
    // when the prior-turn Regidrago V can also obtain its VSTAR card this turn.
    // Reserve Blender while testing every connector so its ACE SPEC copy cannot be
    // counted as a discard cost for the route that is supposed to follow it:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/718
    // https://github.com/FlareZ123/pokemon-sims/pull/931
    Engine without_blender = *this;
    remove_one(without_blender.state_.hand, Card::BrilliantBlender);

    const bool vstar_might_be_searchable = might_be_unseen(Card::RegidragoVstar);
    if (!item_locked() && vstar_might_be_searchable) {
      // Evolution Incense has no discard cost. Mysterious Treasure, Ultra Ball,
      // and Pokémon Communication must retain their exact printed costs after
      // Blender leaves hand:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // https://api.pokemontcg.io/v2/cards/sm9-152
      // https://github.com/FlareZ123/pokemon-sims/issues/718
      if (without_blender.hand_count(Card::EvolutionIncense) > 0) return true;
      if (without_blender.hand_count(Card::MysteriousTreasure) > 0 &&
          without_blender.choose_discard(false, true, true,
                                         Card::MysteriousTreasure).has_value()) {
        return true;
      }
      if (without_blender.payable_ultra_ball_vstar_route()) return true;
      if (without_blender.pokemon_communication_has_direct_target_route(
              Card::RegidragoVstar)) {
        return true;
      }
    }

    const auto has_attached_fss_holder = [this]() {
      if (state_.active && is_pokemon_v(state_.active->card) &&
          state_.active->tool == Tool::ForestSealStone) {
        return true;
      }
      return std::any_of(state_.bench.begin(), state_.bench.end(),
                         [](const Pokemon& pokemon) {
                           return is_pokemon_v(pokemon.card) &&
                               pokemon.tool == Tool::ForestSealStone;
                         });
    };
    const auto has_open_fss_holder = [this]() {
      if (state_.active && is_pokemon_v(state_.active->card) &&
          state_.active->tool == Tool::None) {
        return true;
      }
      return std::any_of(state_.bench.begin(), state_.bench.end(),
                         [](const Pokemon& pokemon) {
                           return is_pokemon_v(pokemon.card) &&
                               pokemon.tool == Tool::None;
                         });
    };
    const bool fss_can_find_vstar = vstar_might_be_searchable;
    const bool fss_can_find_gladion_for_known_prize = supporter_allowed() &&
        prizes_known() && prize_count_after_reveal(Card::RegidragoVstar) > 0 &&
        might_be_unseen(Card::Gladion);
    const bool fss_has_live_vstar_target =
        fss_can_find_vstar || fss_can_find_gladion_for_known_prize;
    const bool held_fss_route = !state_.vstar_power_used &&
        without_blender.hand_count(Card::ForestSealStone) > 0 &&
        has_open_fss_holder() && fss_has_live_vstar_target;
    const bool attached_fss_route = !state_.vstar_power_used &&
        has_attached_fss_holder() && fss_has_live_vstar_target;
    if (held_fss_route || attached_fss_route) {
      // Star Alchemy belongs to Forest Seal Stone, so Rule Box Ability lock does
      // not remove it from the Pokémon V holding the Tool:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
      // https://github.com/FlareZ123/pokemon-sims/issues/718
      return true;
    }

    if (supporter_allowed() && !item_locked() &&
        without_blender.hand_count(Card::Arven) > 0 &&
        vstar_might_be_searchable) {
      Engine after_arven = without_blender;
      remove_one(after_arven.state_.hand, Card::Arven);
      after_arven.state_.supporter_used = true;

      // Arven can obtain Evolution Incense directly, a payable Mysterious
      // Treasure, or Forest Seal Stone for Star Alchemy. The Supporter itself and
      // Blender have already left the projected hand before a discard cost is
      // tested:
      // https://api.pokemontcg.io/v2/cards/sv1-166
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://github.com/FlareZ123/pokemon-sims/issues/718
      if (might_be_unseen(Card::EvolutionIncense)) return true;
      if (might_be_unseen(Card::MysteriousTreasure) &&
          after_arven.choose_discard(false, true, true,
                                     Card::MysteriousTreasure).has_value()) {
        return true;
      }
      if (!state_.vstar_power_used && has_open_fss_holder() &&
          might_be_unseen(Card::ForestSealStone)) {
        return true;
      }
    }

    // Gladion reveals the Prize cards before exchanging itself. K1 requires the
    // VSTAR to be a known Prize, while K0 preserves the public possibility that an
    // unaccounted fixed-list copy is prized:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/718
    const bool direct_gladion_route = supporter_allowed() &&
        without_blender.hand_count(Card::Gladion) > 0 &&
        ((prizes_known() &&
          prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         (!prizes_known() && vstar_might_be_searchable));
    if (direct_gladion_route) return true;

    // Existing recovery policies already prove the complete post-Supporter search
    // costs and exact K1 zone transitions. Copied Engines avoid mutating the real
    // hand, deck, discard, Supporter slot, or RNG while those routes are checked:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_team_yell_vstar_override.inc
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_roseanne_multimode_override.inc
    // https://github.com/FlareZ123/pokemon-sims/issues/718
    Engine team_yell_recovery = without_blender;
    if (team_yell_recovery.play_team_yell_vstar_recovery()) return true;
    Engine roseanne_recovery = without_blender;
    if (roseanne_recovery.play_roseanne_vstar_energy_recovery()) return true;

    return false;
  }

  bool issue_1646_hold_blender_for_burnet_finish_visible() const {
    const bool grass_axis_completes = state_.active &&
        ((!state_.manual_energy_used && state_.active->grass == 1 &&
          hand_count(Card::Grass) > 0) ||
         (state_.manual_energy_used && state_.active->grass >= 2));

    // Professor Burnet already owns the current-turn payload axis whenever the
    // Active Regidrago VSTAR can finish GGF without Brilliant Blender. The hold is
    // therefore a public route-dominance decision, independent of the historical
    // #1646 turn/profile/discard breadcrumbs. StrictJit and MatchupFlexJit both
    // require the payload on the ready turn, and Rule Box Ability lock does not
    // stop either Trainer card. Item and Supporter legality remain semantic gates:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, attachment, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Same-turn JIT and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route and systemic refinement: https://github.com/FlareZ123/pokemon-sims/issues/1646 https://github.com/FlareZ123/pokemon-sims/issues/3010
    return strict_payload_timing() && !item_locked() && prizes_known() &&
        supporter_allowed() && state_.active &&
        state_.active->card == Card::RegidragoVstar &&
        state_.active->fire >= 1 && grass_axis_completes && need_payload() &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 && payload_might_be_in_deck();
  }

  bool play_issue_1700_known_prize_gladion_before_blender() {
    const bool prepared_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass == 1 &&
              pokemon.fire == 0;
        });
    const auto prized_fire =
        std::find(state_.prizes.begin(), state_.prizes.end(), Card::Fire);

    if (!supporter_allowed() || state_.turn != 2 || scenario_.going_first ||
        scenario_.locks != LockMode::None || !prizes_known() ||
        prized_fire == state_.prizes.end() || !prepared_regidrago ||
        hand_count(Card::Gladion) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Grass) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::LatiasEx) == 0 ||
        count_of(state_.discard, Card::EarthenVessel) == 0 ||
        count_of(state_.discard, Card::StevensResolve) == 0 ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }

    // Gladion exchanges itself for one known Prize. Recovering Fire before the T2
    // Grass attachment uses the otherwise idle Supporter slot and preserves T3 Fire
    // plus Brilliant Blender as the earliest complete GGF and current-turn payload:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Prize, Supporter, attachment, evolution, retreat, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, supporter contention, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
    remove_one(state_.hand, Card::Gladion);
    state_.hand.push_back(Card::Fire);
    *prized_fire = Card::Gladion;
    shuffle(state_.prizes);
    state_.supporter_used = true;
    trace("PLAY SUPPORTER",
          "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Looked at the known Prize cards and exchanged Gladion for Fire Energy before the second Grass attachment.");
    return true;
  }

  bool play_brilliant_blender() {
    if (play_issue_1700_known_prize_gladion_before_blender()) return true; // Gladion and confirmed route: https://api.pokemontcg.io/v2/cards/sm4-95 https://github.com/FlareZ123/pokemon-sims/issues/1700

    if (issue_1646_hold_blender_for_burnet_finish_visible()) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1646-hold-blender-for-burnet", state_.turn,
            "HOLD ITEM",
            "R-BLENDER-01; R-BURNET-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
            "Held Brilliant Blender because Professor Burnet completes the current-turn payload after the Vessel-to-Grass route.");
      }
      return false;
    }
    if (!blender_vstar_card_axis_can_finish_this_turn()) return false;
    return play_brilliant_blender_vstar_axis_original();
  }

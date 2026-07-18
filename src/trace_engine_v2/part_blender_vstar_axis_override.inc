  bool blender_vstar_card_axis_can_finish_this_turn() {
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
    // costs and exact K1 zone transitions. A copied Engine avoids mutating the real
    // hand, deck, discard, Supporter slot, or RNG while those routes are checked:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_team_yell_vstar_override.inc
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_roseanne_multimode_override.inc
    // https://github.com/FlareZ123/pokemon-sims/issues/718
    Engine recovery = without_blender;
    if (recovery.play_team_yell_vstar_recovery()) return true;
    recovery = without_blender;
    if (recovery.play_roseanne_vstar_energy_recovery()) return true;

    return false;
  }

  bool play_brilliant_blender() {
    if (!blender_vstar_card_axis_can_finish_this_turn()) return false;
    return play_brilliant_blender_vstar_axis_original();
  }

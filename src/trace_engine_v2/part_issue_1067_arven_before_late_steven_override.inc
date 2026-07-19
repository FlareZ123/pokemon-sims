  bool held_arven_fss_blender_finish_available() const {
    if (!deck_seen_ || !supporter_allowed() || item_locked() ||
        scenario_.dci != DciProfile::StrictJit || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        state_.active->tool != Tool::None || state_.vstar_power_used ||
        hand_count(Card::Arven) == 0 || !need_vstar() || !need_payload() ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !ability_available_for_pokemon(Card::RegidragoV) ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck()) {
      return false;
    }

    const bool forest_seal_stone_available =
        hand_count(Card::ForestSealStone) > 0 ||
        deck_count_after_search_started(Card::ForestSealStone) > 0;
    const bool brilliant_blender_available =
        hand_count(Card::BrilliantBlender) > 0 ||
        deck_count_after_search_started(Card::BrilliantBlender) > 0;

    // Arven can search one Item and one Pokémon Tool. Forest Seal Stone then searches
    // the missing Regidrago VSTAR, the prior-turn Basic may evolve, and Brilliant
    // Blender supplies the strict-JIT Dragon payload during the same turn. Steven's
    // Resolve would end that turn and delay an already complete legal route:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Supporter, Tool, Ability, evolution, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1067
    return forest_seal_stone_available && brilliant_blender_available;
  }

  bool should_play_steven() const {
    if (held_arven_fss_blender_finish_available()) return false;
    return should_play_steven_issue1067_original();
  }

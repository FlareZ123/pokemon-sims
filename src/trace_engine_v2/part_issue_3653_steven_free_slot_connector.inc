    // This continuation is included inside Steven's target loop after the legal
    // deck inspection has established K1. A held Quick Ball plus a Dragon payload
    // is a paid same-ready-turn outlet, so it must not suppress a free second
    // Steven target when Brilliant Blender is known in deck and Steven still has
    // search capacity. Reserving Blender preserves Quick Ball and the held Dragon.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, search, discard, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1 and connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3653
    const bool issue_3653_free_blender_over_paid_quick_ball =
        target == Card::RegidragoVstar && found.empty() && wanted.size() < 3U &&
        payload_outlet == Card::BrilliantBlender &&
        hand_count(Card::BrilliantBlender) == 0 &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
        steven_held_routes_complete_next_turn(
            wanted, next_turn_item_locked, projected_vstar_available);
    if (issue_3653_free_blender_over_paid_quick_ball &&
        move_deck_to_hand(target)) {
      found.push_back(target);
      if (!move_deck_to_hand(Card::BrilliantBlender)) {
        throw std::logic_error("Issue-3653 known Brilliant Blender target disappeared");
      }
      found.push_back(Card::BrilliantBlender);
      continue;
    }

    // This continuation runs inside Steven's legal K1 target loop. A held Quick Ball
    // plus Dragon payload is a paid discard outlet. A free Steven target for Brilliant
    // Blender preserves both cards and is preferred by repository connector-domination policy.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Search/discard procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3653
    const bool free_blender_preserves_paid_quick_ball =
        target == Card::RegidragoVstar && found.empty() && wanted.size() < 3U &&
        payload_outlet == Card::BrilliantBlender &&
        hand_count(Card::BrilliantBlender) == 0 &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
        steven_held_routes_complete_next_turn(
            wanted, next_turn_item_locked, projected_vstar_available);
    if (free_blender_preserves_paid_quick_ball && move_deck_to_hand(target)) {
      found.push_back(target);
      if (!move_deck_to_hand(Card::BrilliantBlender)) {
        throw std::logic_error("Steven free-slot Brilliant Blender target disappeared");
      }
      found.push_back(Card::BrilliantBlender);
      // The already-existing replay flag bypasses only Blender's paid-outlet preservation
      // guard. Steven deliberately banked this free Blender to replace that paid outlet.
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Quick Ball discard cost: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Bug contract: https://github.com/FlareZ123/pokemon-sims/issues/3653
      issue_1912_post_attachment_blender_replay_ = true;
      continue;
    }

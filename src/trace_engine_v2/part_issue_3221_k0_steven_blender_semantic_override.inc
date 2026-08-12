  bool issue_3221_k0_steven_blender_route_available() const {
    // TurnTwoItem locks Items from the player's second turn onward, so any projected
    // Blender on T2+ is illegal: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Brilliant Blender is an Item: https://api.pokemontcg.io/v2/cards/sv8-164
    const bool next_turn_items_locked =
        scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        (scenario_.locks == LockMode::TurnTwoItem && state_.turn + 1 >= 2);

    // This is specifically the K0 variant. Any legal K1 source, including a complete
    // Prize inspection, leaves this helper and is handled by the known-composition route.
    // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // At K0, Steven's Resolve may establish the first physical deck inspection and
    // reserve the plausible VSTAR plus final Grass. Brilliant Blender is held for
    // the following same-ready-turn payload discard. Rule Box Ability lock does not
    // affect either Trainer; Item locks matter on the projected Blender turn.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Supporter, Item, search, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3221
    if (prizes_known() || !strict_payload_timing() || next_turn_items_locked ||
        state_.turn >= scenario_.max_turn || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 || hand_count(Card::Grass) > 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        !payload_might_be_in_deck()) {
      return false;
    }

    // The active prior-turn Regidrago V is one Grass short of GGF. Steven can search
    // the plausible VSTAR and Grass, end the turn, then the held Blender supplies the
    // Dragon payload after the searched attachment and legal evolution next turn.
    // The route depends on state semantics rather than an absolute turn/profile/lock
    // witness; hidden targets remain conditional until Steven resolves the K0 search.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3221
    return grass_needed() == 1 && fire_needed() == 0 &&
        might_be_unseen(Card::RegidragoVstar) && might_be_unseen(Card::Grass);
  }

  bool late_steven_vstar_vessel_route_available() const {
    const bool next_turn_item_locked =
        scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        (uses_turn_two_item_lock(scenario_.locks) && state_.turn + 1 >= 2);

    // Steven ends the current turn after reserving VSTAR/Vessel. On the following
    // turn the prior-turn Regidrago V may evolve and Earthen Vessel pays its printed
    // discard cost with the held Dragon payload on the actual ready turn. Both JIT
    // profiles therefore share this route. Rule Box Ability lock is irrelevant
    // because the continuation uses Trainer actions and evolution, while Item locks
    // still block the projected Vessel play.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, discard, search, evolution, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1/JIT/lock/route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3202
    if (!prizes_known() || state_.turn < 2 || state_.turn >= scenario_.max_turn ||
        !strict_payload_timing() || next_turn_item_locked || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        !pays_apex_energy_cost(*state_.active) || !need_vstar() ||
        !need_payload() || hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        (hand_count(Card::EarthenVessel) == 0 &&
         deck_count_after_search_started(Card::EarthenVessel) == 0) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    return deck_count_after_search_started(Card::Grass) > 0 ||
           deck_count_after_search_started(Card::Fire) > 0;
  }

  Card choose_supporter_after_search_started() {
    if (late_steven_vstar_vessel_route_available() &&
        deck_count_after_search_started(Card::StevensResolve) > 0) {
      // Wonder Tag searches one Supporter. Steven is the complete connector because
      // it reserves the missing VSTAR/Vessel package; a payload-only Supporter leaves
      // the Evolution axis unresolved.
      // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Connector-priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3202
      return Card::StevensResolve;
    }
    return choose_supporter_after_search_started_issue3202_const_fallback();
  }

  bool should_play_steven_issue1030_original() const {
    return late_steven_vstar_vessel_route_available() ||
        should_play_steven_issue3202_original();
  }

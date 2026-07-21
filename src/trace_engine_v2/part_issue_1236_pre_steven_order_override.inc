  void play_items_until_stable(const bool permit_payload) {
    // Resolve the payable Energy search and T1 attachment window before the
    // turn-ending Steven's Resolve package. The route guard proves that both
    // printed discard costs and every later searched axis remain available:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Official Item, attachment, Supporter, and turn-ending procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1236
    if (!state_.turn_ended && issue_1236_pre_steven_vessel_route_available()) {
      play_earthen_vessel(false);
    }
    play_items_until_stable_issue1236_original(permit_payload);
  }

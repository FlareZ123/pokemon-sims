  bool play_quick_ball(const bool permit_payload) {
    if (play_quick_ball_issue1237_original(permit_payload)) return true;
    const auto cost = quick_ball_dialga_vessel_first_cost();
    if (!cost) return false;

    // Reuse the exact cost proven by the K1 payload-outlet preflight, then search
    // Basic Dialga-GX. The separately held Earthen Vessel can immediately discard
    // Dialga-GX and complete strict-JIT readiness on this turn:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Official Item, discard, and deck-search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, earliest-route, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1237
    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(*cost, "Quick Ball route-conditioned cost", "R-QB-01; P-DCI-01");
    record_deck_search_knowledge("Quick Ball");
    if (!move_deck_to_hand(Card::DialgaGX)) {
      throw std::logic_error("projected issue-1237 Dialga-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Paid the K1-proven first cost and searched Dialga-GX for the held Earthen Vessel outlet.");
    return true;
  }

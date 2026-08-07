  bool issue_2265_arven_quick_ball_surplus_route_live() const {
    if (!prizes_known() || !supporter_allowed() ||
        hand_count(Card::Arven) == 0 || item_locked() ||
        deck_count_after_search_started(Card::QuickBall) == 0 ||
        hand_count(Card::QuickBall) > 0) {
      return false;
    }

    // Arven consumes the turn's Supporter permission, so only a non-Supporter
    // payload outlet may complete the post-search route. Brilliant Blender is held,
    // legal this turn, and a permitted Dragon payload is known to remain in deck:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // One-Supporter-per-turn and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    const bool blender_payload_route =
        hand_count(Card::BrilliantBlender) > 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck();
    if (!blender_payload_route) return false;

    // The existing Quick Ball classifier proves the rest of the exact observable
    // continuation: selected Benched VSTAR already has GGF, the Active is Basic,
    // retreat is unused, Latias ex is physically available, Skyliner is legal,
    // Bench space exists, and one held Basic Energy is dynamically surplus. Calling
    // it before Arven is safe because the helper does not require Quick Ball in hand;
    // this gate separately proves Arven can fetch that physical Item from K1 deck.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core search, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing direct-held-Quick-Ball route: https://github.com/FlareZ123/pokemon-sims/issues/1185
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    return quick_ball_final_surplus_energy_cost().has_value();
  }

  bool play_arven() {
    if (!issue_2265_arven_quick_ball_surplus_route_live()) {
      return play_arven_issue2265_original();
    }

    // Search only the Item required by the proven continuation. Arven says to search
    // for an Item and a Pokémon Tool; a typed deck search may choose fewer matching
    // cards, so the Tool branch can legally fail when it advances no setup axis:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Typed-search voluntary failure ruling: https://compendium.pokegym.net/compendium-bw.html#trainers-in-general
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2265 route");
    const bool found = move_deck_to_hand(Card::QuickBall);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01; P-COMPRESS-01",
          found
              ? "Arven searched Quick Ball for the final surplus-Energy Latias/Blender route."
              : "Arven legally inspected the deck, but the projected Quick Ball target disappeared.");
    return true;
  }

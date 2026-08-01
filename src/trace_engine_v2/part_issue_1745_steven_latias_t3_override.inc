  bool issue_1745_steven_latias_t3_route_available() const {
    // A legal deck inspection and a complete Prize inspection both establish K1
    // for the fixed-list route; all physical timing, lock, target, and board gates
    // remain explicit below:
    // Hisuian Heavy Ball Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Official Prize, search, evolution, attachment, Bench, Ability, Retreat, and Supporter procedure: https://assets.pokemon.com/assets/cms2-en-uk/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2061
    if (!prizes_known() || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        scenario_.max_turn < 3 || !supporter_allowed() ||
        !state_.manual_energy_used || !state_.active ||
        !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        in_play(Card::LatiasEx) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::LatiasEx) > 0 || hand_count(Card::Grass) > 0 ||
        has_vstar() || !need_vstar() || !payload_ready()) {
      return false;
    }

    const auto prepared_regi =
        std::find_if(state_.bench.begin(), state_.bench.end(),
                     [this](const Pokemon& pokemon) {
                       return pokemon.card == Card::RegidragoV &&
                              pokemon.entered_turn < state_.turn &&
                              pokemon.grass == 1 && pokemon.fire == 1;
                     });
    if (prepared_regi == state_.bench.end()) return false;

    // Steven searches the missing VSTAR, Latias ex, and final Grass before ending T2.
    // The prior-turn Regidrago evolves on T3, receives Grass, and Skyliner gives the
    // Basic Active zero Retreat Cost. The banked Dragon already satisfies the
    // no-discard-control payload axis, so every missing component is deterministic:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX Basic identity: https://api.pokemontcg.io/v2/cards/sm2-60
    // Official Supporter, evolution, attachment, Ability, Retreat, and turn procedure: https://assets.pokemon.com/assets/cms2-en-uk/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, no-discard-control payload, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed route bug: https://github.com/FlareZ123/pokemon-sims/issues/1745
    // Confirmed K1 provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2061
    return deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0;
  }

  bool play_issue_1745_steven_latias_t3_route() {
    if (!issue_1745_steven_latias_t3_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1745 route");

    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::LatiasEx) ||
        !move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-1745 Steven targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven permits up to three unrestricted targets and then ends the turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official search and turn-ending procedure: https://assets.pokemon.com/assets/cms2-en-uk/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed route bug: https://github.com/FlareZ123/pokemon-sims/issues/1745
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the deterministic T3 Latias route: Regidrago VSTAR, Latias ex, Grass Energy. Turn ended.");
    return true;
  }

  bool should_play_steven_issue1745() const {
    return issue_1745_steven_latias_t3_route_available() ||
        should_play_steven();
  }

  bool play_steven_issue1745() {
    if (play_issue_1745_steven_latias_t3_route()) return true;
    return play_steven();
  }
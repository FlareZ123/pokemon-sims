  bool issue_1771_steven_t4_package_available() const {
    const bool prepared_benched_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
              pokemon.fire >= 1;
        });
    const bool payload_in_known_deck =
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);

    if (!deck_seen_ || !strict_payload_timing() || !supporter_allowed() ||
        state_.turn != 3 || state_.turn + 1 > scenario_.max_turn ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || state_.retreat_used ||
        bench_space() == 0 || !prepared_benched_regidrago ||
        !need_vstar() || !need_payload() || need_energy() ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx) ||
        hand_count(Card::ProfessorBurnet) > 0 ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }

    // K1 proves that Steven can bank every missing next-turn axis. The prior-turn
    // GGF Regidrago V evolves on T4, Latias ex gives the Basic Active no Retreat
    // Cost, and Professor Burnet supplies a same-turn strict-JIT Dragon payload.
    // Tate & Liza draw mode would replace this deterministic package with five
    // random cards and therefore has lower active-move realism:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, evolution, Bench, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, active-move realism, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1771
    return deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        payload_in_known_deck;
  }

  bool play_issue_1771_steven_t4_package() {
    if (!issue_1771_steven_t4_package_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1771 route");

    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::LatiasEx) ||
        !move_deck_to_hand(Card::ProfessorBurnet)) {
      throw std::logic_error("Issue-1771 Steven target disappeared");
    }
    shuffle(state_.deck);

    // Steven searches exactly the evolution, promotion, and strict-JIT payload
    // channels that are publicly known to remain. Its printed "up to 3" text
    // permits this exact package and ends the turn after the search:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // https://github.com/FlareZ123/pokemon-sims/issues/1771
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the deterministic T4 VSTAR-Latias-Burnet package. Turn ended.");
    return true;
  }

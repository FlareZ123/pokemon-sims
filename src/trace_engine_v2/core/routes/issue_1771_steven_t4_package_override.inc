  bool issue_1771_steven_t4_package_available() const {
    const int current_turn = state_.turn;
    const bool prepared_benched_regidrago = bench_has_pokemon_matching(
        [this, current_turn](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon_entered_before_turn(pokemon, current_turn) &&
              pays_apex_energy_cost(pokemon);
              // https://github.com/FlareZ123/pokemon-sims/issues/2424
              // Canonical Bench/evolution query owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
        });
    const bool payload_in_known_deck = payload_zone_contains(state_.deck);
    // Shared payload membership policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
    // Apex Dragon payload semantics: https://api.pokemontcg.io/v2/cards/swsh12-136

    // Complete legal deck or Prize inspection establishes K1:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Manual procedure source: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed provenance defect: https://github.com/FlareZ123/pokemon-sims/issues/2065
    if (!prizes_known() || !strict_payload_timing() || !supporter_allowed() ||
        state_.turn + 1 > scenario_.max_turn ||
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
    // GGF Regidrago V evolves on the following turn, Latias ex gives the Basic
    // Active no Retreat Cost, and Professor Burnet supplies a same-turn strict-JIT
    // Dragon payload. Tate & Liza draw mode would replace this deterministic package
    // with five random cards and therefore has lower active-move realism:
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
    const int completion_turn = state_.turn + 1;
    // Steven ends the current turn after the search, so the route witness is the
    // immediately following simulator turn rather than an absolute T4 label:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed relative-trace bug: https://github.com/FlareZ123/pokemon-sims/issues/3798
    // Sibling relative-turn route implementation:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_1772_steven_t3_package_override.inc
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the deterministic T" + std::to_string(completion_turn) +
              " VSTAR-Latias-Burnet package. Turn ended.");
    return true;
  }
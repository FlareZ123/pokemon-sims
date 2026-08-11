  bool issue_3040_play_turo_regidrago_staging_route() {
    if (!issue_3040_professor_turo_is_unique_active_escape() ||
        !strict_payload_timing() || !payload_ready() || !state_.active ||
        !is_basic(state_.active->card) || state_.active->tool != Tool::None ||
        !need_active_vstar() || need_regi() || !need_vstar() || need_energy() ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0) {
      return false;
    }

    const auto promoted = std::find_if(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn &&
                 pays_apex_energy_cost(pokemon);
        });
    if (promoted == state_.bench.end()) return false;

    // K1 has already established that Latias ex cannot supply the free-Retreat
    // connector and that a Regidrago VSTAR remains in the deck. The zero-resource
    // Basic Active cannot pay its printed Retreat Cost, while the prior-turn
    // Benched Regidrago V already pays GGF. Turo can therefore advance the public
    // Active-position axis now, during an otherwise unused Supporter action, without
    // consuming the later VSTAR search or the protected Dragon needed for a future
    // same-ready-turn JIT payload event.
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Active replacement, Retreat, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1, strict-JIT, and resource-priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
    const Pokemon returned = *state_.active;
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;
    state_.hand.push_back(returned.card);
    state_.active = *promoted;
    state_.bench.erase(promoted);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-DCI-01",
          "Professor Turo returned the zero-resource Basic Active and promoted the powered prior-turn Regidrago V for its observable VSTAR staging route.");
    return true;
  }

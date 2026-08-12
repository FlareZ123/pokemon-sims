  bool late_steven_burnet_route_available() const {
    // This route uses Steven's Resolve on the current turn, then ordinary evolution,
    // one manual Energy attachment, and the already-held Professor Burnet next turn.
    // Its legality depends on same-ready-turn JIT and Supporter availability. Item and
    // Rule Box Ability lock states do not touch any action used by the route.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, evolution, attachment, search, discard, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT profiles: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Action-specific lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed semantic-profile/lock bug: https://github.com/FlareZ123/pokemon-sims/issues/3223
    if (!prizes_known() || state_.turn >= scenario_.max_turn ||
        !strict_payload_timing() || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        !has_payload_in_hand() ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    // Steven ends the present turn, so the route also proves that the projected
    // next turn permits Professor Burnet before admitting the package.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Supporter timing policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
    const PublicState next_public =
        advance_public_state_end_of_turn(project_public_state());
    return next_public.supporter_allowed;
  }

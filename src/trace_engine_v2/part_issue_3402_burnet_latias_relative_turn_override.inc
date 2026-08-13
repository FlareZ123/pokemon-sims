  bool issue_2343_burnet_quick_ball_latias_route_available() const {
    const bool burnet_available = hand_count(Card::ProfessorBurnet) > 0 ||
        deck_count_after_search_started(Card::ProfessorBurnet) > 0;
    const bool deck_payload = std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    const bool held_payload = std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool blender_payload_route = hand_count(Card::BrilliantBlender) > 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck();
    const bool held_manual_completes_energy = !state_.manual_energy_used &&
        ((grass_needed() == 1 && fire_needed() == 0 && hand_count(Card::Grass) > 0) ||
         (grass_needed() == 0 && fire_needed() == 1 && hand_count(Card::Fire) > 0));
    const bool energy_axis_complete = benched_vstar_promotion_ready();

    // Card text and Trainer procedure impose no absolute turn-3 requirement. The
    // route is admitted by current legal actions, K1, same-ready-turn JIT, and horizon.
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Item/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L440
    // K1/JIT/priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3402
    return strict_payload_timing() && scenario_.locks == LockMode::None &&
        state_.turn <= scenario_.max_turn && prizes_known() && supporter_allowed() &&
        !item_locked() && !state_.retreat_used && state_.active &&
        is_basic(state_.active->card) && need_active_vstar() && need_payload() &&
        can_play_payload_this_turn() && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 && deck_count_after_search_started(Card::LatiasEx) > 0 &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::StevensResolve) > 0 &&
        burnet_available && deck_payload && !held_payload && !blender_payload_route &&
        (energy_axis_complete || held_manual_completes_energy);
  }

  Card choose_supporter_after_search_started() const {
    const Card legacy = choose_supporter_after_search_started_issue3402_legacy();
    if (!issue_2343_burnet_quick_ball_latias_route_available() ||
        deck_count_after_search_started(Card::ProfessorBurnet) == 0) return legacy;

    // These legacy results correspond to selector branches that precede #2343 and
    // therefore keep priority. All other legacy results occur after the #2343 branch.
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3402
    if (legacy == Card::StevensResolve || legacy == Card::Gladion || legacy == Card::Dawn) {
      return legacy;
    }
    return Card::ProfessorBurnet;
  }

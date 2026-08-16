  bool late_steven_vstar_vessel_route_available() const {
    const bool next_turn_item_locked = item_locked_on_turn(state_.turn + 1);
    // Shared projected Item-lock timing: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc

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
        !payload_zone_contains(state_.hand)) {
      // Canonical Dragon-payload membership: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
      return false;
    }

    return deck_count_after_search_started(Card::Grass) > 0 ||
           deck_count_after_search_started(Card::Fire) > 0;
  }

  std::optional<int> issue_3458_1797_projected_finish_turn() const {
    // The #1797 connector needs an unused Supporter action. When Steven's Resolve is
    // legal now, it may be found by Wonder Tag and played during this turn, then the
    // semantic #3316 continuation finishes on the immediately following player turn.
    // The first player cannot play a Supporter during that player's first turn, so
    // that one rules-defined state banks Steven until the next turn and finishes one
    // turn later. An already-spent Supporter action is intentionally not treated as
    // another bank case because Supporter contention is part of route availability.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Advanced Supporter and first-turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Repository Supporter-contention and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    if (state_.supporter_used) return std::nullopt;

    int steven_turn = state_.turn;
    if (!supporter_allowed()) {
      const bool first_turn_bank = scenario_.going_first && state_.turn == 1 &&
          supporter_rules_allow_turn(state_.turn + 1);
      if (!first_turn_bank) return std::nullopt;
      steven_turn = state_.turn + 1;
    }

    const int finish_turn = steven_turn + 1;
    // Earthen Vessel is required on the finish turn. Project the actual Item lock
    // instead of requiring a historical no-lock scenario identity.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Turn-two Item-lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Semantic continuation: https://github.com/FlareZ123/pokemon-sims/issues/3316
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    if (finish_turn > scenario_.max_turn || item_locked_on_turn(finish_turn)) {
      return std::nullopt;
    }
    return finish_turn;
  }

  bool issue_3458_1797_connector_common_available() const {
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);

    // This is the shared semantic contract for both sides of the physical Quick Ball
    // transaction. K1 may come from any legal deck or Prize inspection. Quick Ball
    // must be legal now, Wonder Tag must be available now, and the held VSTAR plus
    // two-Grass package and known Steven/Crispin/Vessel/Energy/payload resources must
    // survive into the relative #3316 bank/finish continuation.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Item, Ability, search, Bench, attachment, evolution, and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1/JIT/lock/route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Semantic downstream continuation: https://github.com/FlareZ123/pokemon-sims/issues/3316
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return strict_payload_timing() && prizes_known() && !item_locked() &&
        issue_3458_1797_projected_finish_turn().has_value() &&
        !state_.manual_energy_used && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        state_.active->double_dragon == 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_grass > 0 && deck_fire > 0 && deck_grass + deck_fire >= 5 &&
        deck_payload;
  }

  bool issue_3458_1797_quick_ball_tapu_steven_route_available() const {
    const bool legal_discard_cost = hand_count(Card::TateLiza) > 0 ||
        hand_count(Card::Lusamine) > 0;

    // The pre-search stage owns only the physical Quick Ball/Tapu requirements.
    // K1 provenance is deliberately absent: prizes_known() in the shared contract
    // is sufficient regardless of which legal inspection established it.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return issue_3458_1797_connector_common_available() &&
        hand_count(Card::QuickBall) > 0 && legal_discard_cost &&
        bench_space() > 0 && hand_count(Card::TapuLeleGX) == 0 &&
        !in_play(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0;
  }

  bool issue_3458_1797_wonder_tag_steven_route_available() const {
    const bool route_cost_paid = count_of(state_.discard, Card::TateLiza) > 0 ||
        count_of(state_.discard, Card::Lusamine) > 0;

    // After Quick Ball resolves, the same semantic contract is reused. Only the
    // transaction provenance changes: Quick Ball and its approved DCI cost are now
    // in discard and the searched Tapu Lele-GX is in play resolving Wonder Tag.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return issue_3458_1797_connector_common_available() &&
        count_of(state_.discard, Card::QuickBall) > 0 && route_cost_paid &&
        in_play(Card::TapuLeleGX);
  }

  // The legacy non-const selector was macro-renamed for this wrapper. Its internal
  // fallback explicitly casts to const, so preserve the matching const overload and
  // forward that call to the pre-wrapper selector.
  // C++ member overload lookup: https://eel.is/c++draft/class.member.lookup
  Card choose_supporter_after_search_started_issue3202_original() const {
    return choose_supporter_after_search_started_issue3202_const_fallback();
  }

  Card choose_supporter_after_search_started() {
    if (issue_3458_1797_wonder_tag_steven_route_available()) {
      // Wonder Tag searches Steven under the same semantic contract that admitted
      // the preceding Quick Ball, including the relative bank/finish horizon.
      // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
      return Card::StevensResolve;
    }
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
    return choose_supporter_after_search_started_issue3202_original();
  }

  bool should_play_steven_issue1030_original() const {
    return late_steven_vstar_vessel_route_available() ||
        should_play_steven_issue3202_original();
  }
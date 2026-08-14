  bool late_steven_vstar_vessel_route_available() const {
    const bool next_turn_item_locked =
        scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        (uses_turn_two_item_lock(scenario_.locks) && state_.turn + 1 >= 2);

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
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    return deck_count_after_search_started(Card::Grass) > 0 ||
           deck_count_after_search_started(Card::Fire) > 0;
  }

  bool issue_3458_1797_current_steven_bank_state_available() const {
    // The generalized current-Supporter case reuses the already-proven #3316 bank:
    // Regidrago V has survived from a previous turn with one Grass, the manual
    // attachment is still unused, and a second held Grass can be attached before
    // Steven's Resolve ends this turn. These physical gates prevent a zero-Energy
    // state from projecting an impossible next-turn GGF finish.
    // Regidrago V / VSTAR and GGF: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Advanced evolution, attachment, and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Semantic bank/finish continuation: https://github.com/FlareZ123/pokemon-sims/issues/3316
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return supporter_allowed() && !state_.manual_energy_used && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 0 &&
        state_.active->double_dragon == 0 && hand_count(Card::Grass) > 0;
  }

  bool issue_3458_1797_first_player_t1_bank_state_available() const {
    // Preserve #1797's original first-player T1 bank. Wonder Tag may search Steven,
    // but the first player cannot play a Supporter on that first turn. The held two
    // Grass preserve the T1 and T2 manual attachments before #3316 finishes on T3.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Official first-turn Supporter rule: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced first-turn and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1797
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return !supporter_allowed() && scenario_.going_first && state_.turn == 1 &&
        supporter_rules_allow_turn(state_.turn + 1) && !state_.manual_energy_used &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        state_.active->double_dragon == 0 && hand_count(Card::Grass) >= 2;
  }

  std::optional<int> issue_3458_1797_projected_finish_turn() const {
    if (state_.supporter_used) return std::nullopt;

    int finish_turn = 0;
    if (issue_3458_1797_current_steven_bank_state_available()) {
      finish_turn = state_.turn + 1;
    } else if (issue_3458_1797_first_player_t1_bank_state_available()) {
      finish_turn = state_.turn + 2;
    } else {
      return std::nullopt;
    }

    // Earthen Vessel is the payload outlet on the ready turn, so project the real
    // Item legality of that turn instead of requiring LockMode::None globally.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Projected lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return finish_turn <= scenario_.max_turn && !item_locked_on_turn(finish_turn)
        ? std::optional<int>{finish_turn}
        : std::nullopt;
  }

  bool issue_3458_1797_connector_common_available() const {
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const bool deck_payload =
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);

    // K1 may come from any legal deck or Prize inspection. The connector is governed
    // by current Item legality, current Tapu Rule Box Ability legality, the relative
    // Steven bank/finish horizon, and the package resources that #1797/#3316 consume.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Item, Ability, search, Bench, attachment, evolution, and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1/JIT/lock/route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original and semantic continuation: https://github.com/FlareZ123/pokemon-sims/issues/1797 https://github.com/FlareZ123/pokemon-sims/issues/3316
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return strict_payload_timing() && prizes_known() && !item_locked() &&
        issue_3458_1797_projected_finish_turn().has_value() &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        hand_count(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_grass > 0 && deck_fire > 0 && deck_grass + deck_fire >= 5 &&
        deck_payload;
  }

  bool issue_3458_1797_quick_ball_tapu_steven_route_available() const {
    const bool legal_discard_cost = hand_count(Card::TateLiza) > 0 ||
        hand_count(Card::Lusamine) > 0;

    // The pre-search side owns only the physical Quick Ball/Tapu requirements. K1
    // provenance is intentionally absent because prizes_known() already proves the
    // repository knowledge state. Preserve the confirmed Tate & Liza/Lusamine DCI
    // payment set and the protected VSTAR/Energy package.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    return issue_3458_1797_connector_common_available() &&
        hand_count(Card::QuickBall) > 0 && legal_discard_cost &&
        bench_space() > 0 && hand_count(Card::TapuLeleGX) == 0 &&
        !in_play(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0;
  }

  std::optional<Card> issue_3458_1797_quick_ball_cost() const {
    if (!issue_3458_1797_quick_ball_tapu_steven_route_available()) {
      return std::nullopt;
    }
    // Preserve #1797's confirmed dynamic-DCI payment order without binding it to
    // seed coordinates. The connector replaces the next-turn Supporter role of the
    // paid card, while VSTAR and required Grass remain protected.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed connector bug: https://github.com/FlareZ123/pokemon-sims/issues/3458
    if (hand_count(Card::TateLiza) > 0) return Card::TateLiza;
    return hand_count(Card::Lusamine) > 0
        ? std::optional<Card>{Card::Lusamine}
        : std::nullopt;
  }

  bool issue_3458_1797_wonder_tag_steven_route_available() const {
    const bool route_cost_paid = count_of(state_.discard, Card::TateLiza) > 0 ||
        count_of(state_.discard, Card::Lusamine) > 0;

    // After Quick Ball resolves, the same semantic contract applies. Only transaction
    // provenance changes: Quick Ball and the approved DCI cost are in discard, while
    // Tapu Lele-GX is in play and Wonder Tag is resolving.
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
    if (issue_3458_1797_wonder_tag_steven_route_available() &&
        deck_count_after_search_started(Card::StevensResolve) > 0) {
      // Wonder Tag chooses Steven under the same semantic contract that admitted
      // Quick Ball, including the relative bank/finish and current Ability legality.
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

         deck_count_after_search_started(Card::Fire) == 0)) {
      return false;
    }

    // Evolve before resolving the held Vessel so the Dragon discard is permitted by
    // strict-JIT on the same turn that Regidrago VSTAR becomes attack-ready. Vessel
    // still performs its printed Basic Energy search after paying that discard cost:
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Core evolution and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1017
    evolve_best_regi();
    if (!active_is_vstar() || !play_earthen_vessel(true)) {
      throw std::logic_error("Known late-Steven VSTAR-Vessel continuation failed");
    }
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
    return true;
  }


bool complete_issue_1599_quick_ball_tapu_crispin_route() {
  if (scenario_.dci != DciProfile::MatchupFlexJit || state_.turn != 2 ||
      item_locked() || !supporter_allowed() || !deck_seen_ ||
      !state_.active || state_.active->card != Card::RegidragoVstar ||
      state_.active->grass < 2 || state_.active->fire != 0 ||
      need_regi() || need_vstar() || need_active_vstar() || need_payload() ||
      hand_count(Card::QuickBall) == 0 || hand_count(Card::GoodraVstar) == 0 ||
      bench_space() == 0 || !ability_available_for_pokemon(Card::TapuLeleGX) ||
      in_play(Card::TapuLeleGX) ||
      deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
      deck_count_after_search_started(Card::Crispin) == 0 ||
      deck_count_after_search_started(Card::Grass) == 0 ||
      deck_count_after_search_started(Card::Fire) == 0) {
    return false;
  }

  // Reopen the costed Basic-search channel after Mysterious Treasure has
  // established the VSTAR and current-turn payload axes. Goodra is then a
  // route-proven high-DCI cost, Tapu Lele-GX supplies Wonder Tag, and Crispin
  // attaches the sole missing Fire Energy during the unused Supporter action:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, discard, search, Bench, Ability, Supporter, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1599
  remove_one(state_.hand, Card::QuickBall);
  state_.discard.push_back(Card::QuickBall);
  if (!discard_from_hand(Card::GoodraVstar,
                         "Quick Ball issue-1599 Tapu-Crispin route cost",
                         "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
    throw std::logic_error("Issue-1599 Quick Ball cost disappeared");
  }
  record_deck_search_knowledge("Quick Ball issue-1599 Tapu-Crispin route");
  if (!move_deck_to_hand(Card::TapuLeleGX)) {
    throw std::logic_error("Issue-1599 Tapu Lele-GX target disappeared");
  }
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-DCI-01",
        "Spent Hisuian Goodra VSTAR and searched Tapu Lele-GX for the immediate Crispin finish.");

  if (!bench_from_hand(Card::TapuLeleGX, false)) {
    throw std::logic_error("Issue-1599 Tapu Lele-GX Bench action failed");
  }
  record_deck_search_knowledge("Wonder Tag issue-1599 Crispin route");
  if (!move_deck_to_hand(Card::Crispin)) {
    throw std::logic_error("Issue-1599 Crispin target disappeared");
  }
  shuffle(state_.deck);
  trace("WONDER TAG", "R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
        "Searched and revealed Crispin for the missing Fire Energy axis.");
  if (!play_crispin()) {
    throw std::logic_error("Issue-1599 Crispin completion disappeared");
  }
  return active_is_vstar() && !need_energy() && !need_payload();
}

bool complete_issue_1597_banked_steven_turn() {
  if (scenario_.dci != DciProfile::NoDiscardControl ||
      !scenario_.going_first || scenario_.locks != LockMode::None ||
      state_.turn != 2 || !supporter_allowed() || state_.manual_energy_used ||
      !deck_seen_ || !state_.active || state_.active->card != Card::Oricorio ||
      !std::any_of(state_.discard.begin(), state_.discard.end(), is_payload) ||
      hand_count(Card::StevensResolve) == 0 || hand_count(Card::Fire) == 0 ||
      hand_count(Card::ProfessorTuro) == 0 || !need_vstar() || !need_energy() ||
      deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
      deck_count_after_search_started(Card::Crispin) == 0) {
    return false;
  }

  // The T1 Wonder Tag banked Steven because the first player could not play a
  // Supporter. Attach the held Fire, then Steven searches the missing VSTAR and
  // the next-turn Crispin while preserving the later Active-position connector:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, search, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Supporter schedule and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1597
  attach_manual();
  remove_one(state_.hand, Card::StevensResolve);
  state_.discard.push_back(Card::StevensResolve);
  state_.supporter_used = true;
  record_deck_search_knowledge("Steven's Resolve issue-1597 schedule");
  if (!move_deck_to_hand(Card::RegidragoVstar) ||
      !move_deck_to_hand(Card::Crispin)) {
    throw std::logic_error("Issue-1597 Steven targets disappeared");
  }
  shuffle(state_.deck);
  state_.turn_ended = true;
  trace("PLAY SUPPORTER", "R-STEVEN-01; P-COMPRESS-01",
        "Steven's Resolve banked Regidrago VSTAR and Crispin for the next Energy turn; the turn ended.");
  return true;
}

#include "part_issue_1751_crobat_compression.inc"

  bool issue_1751_recipe_has_crobat() const {
    // Keep the registered shell and Pineco action selectors on the authoritative
    // legacy policy. This replacement turn policy is recipe-gated to experimental
    // Crobat V lists, matching every other Crobat modeling decision:
    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Modeling boundary: https://github.com/FlareZ123/pokemon-sims/issues/1394
    // Confirmed sequencing bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
    return std::any_of(recipe_.begin(), recipe_.end(), [](const auto& entry) {
      return entry.first == Card::CrobatV && entry.second > 0;
    });
  }

void run_turn() {
  if (!issue_1751_recipe_has_crobat()) {
    run_turn_original();
    return;
  }
  if (secret_box_combo_enabled()) {
    run_secret_box_turn();

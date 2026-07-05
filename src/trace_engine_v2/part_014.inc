      if (bench_oricorio_if_useful()) return true;
      if (play_earthen_vessel(permit_payload)) return true;
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
    }
    if (permit_payload && need_payload()) {
      if (play_brilliant_blender()) return true;
      // MT/QB are allowed to build a two-connector discard chain in this policy.
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_quick_ball(permit_payload)) return true;
      if (play_earthen_vessel(permit_payload)) return true;
    }
    return false;
  }

  void play_items_until_stable(const bool permit_payload) {
    if (item_locked() || state_.turn_ended) return;
    for (int guard = 0; guard < 28; ++guard) {
      if (!run_search_items_one_step(permit_payload)) break;
    }
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (need_active_vstar() && !need_energy() && !need_payload() && play_tate_switch()) return;
    if (play_gladion()) return;
    if (play_steven()) return;
    if (need_energy() && hand_count(Card::Crispin) > 0 && play_crispin()) return;
    if (need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) > 0 && play_professor_burnet()) return;
    if (hand_count(Card::Arven) > 0 && play_arven()) return;
    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::BrilliantBlender) == 0 && play_professor_burnet()) return;
    if (hand_count(Card::Serena) > 0 && play_serena()) return;
    if (need_active_vstar() && play_tate_switch()) return;
    play_tate_draw();
  }

  void run_turn() {
    trace("POLICY", "P-AXIS-01", "Start: " + state_line());
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());
    attach_fss();
    use_fss();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
    choose_supporter();
    if (state_.turn_ended) {
      trace("TURN END", "R-STEVEN-01 or R-RV-01", state_line());
      return;
    }

    play_basics_from_hand();
    attach_fss();
    use_fss();
    play_items_until_stable(!strict_payload_timing());
    attach_manual();
    evolve_best_regi();
    play_basics_from_hand();
    play_items_until_stable(true);
    use_legacy_star();
    attach_manual();
    retreat_to_benched_vstar_with_latias();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
  }

  void record_ready() {
    const bool ready = state_.turn >= 2 && active_is_vstar() && state_.active->grass >= 2 && state_.active->fire >= 1 && payload_ready();
    if (!ready) return;
    if (outcome_.first_ready_turn == 0) {
      outcome_.first_ready_turn = state_.turn;
      trace("READY", "R-RVS-01; P-JIT-01", "Active Regidrago VSTAR has GGF and a permitted Dragon payload in discard.");
    }
    if (state_.turn <= 2) outcome_.ready_by_2 = true;
    if (state_.turn <= 3) outcome_.ready_by_3 = true;
    if (state_.turn <= 4) outcome_.ready_by_4 = true;
  }
};

DeckRecipe baseline_recipe() { return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end()); }

std::vector<Scenario> all_scenarios() {
  std::vector<Scenario> result;
  for (const bool first : {true, false}) {
    const std::string seat = first ? "go-first" : "go-second";
    result.push_back({"strict-jit/" + seat, DciProfile::StrictJit, LockMode::None, first, 4});
    result.push_back({"matchup-flex-jit/" + seat, DciProfile::MatchupFlexJit, LockMode::None, first, 4});
    result.push_back({"no-discard-control/" + seat, DciProfile::NoDiscardControl, LockMode::None, first, 4});
    result.push_back({"strict-jit-turn2-item-lock/" + seat, DciProfile::StrictJit, LockMode::TurnTwoItem, first, 4});
    result.push_back({"strict-jit-full-item-lock/" + seat, DciProfile::StrictJit, LockMode::FullItem, first, 4});
    result.push_back({"strict-jit-rulebox-ability-lock/" + seat, DciProfile::StrictJit, LockMode::FullRuleBoxAbility, first, 4});
    result.push_back({"strict-jit-combined-lock/" + seat, DciProfile::StrictJit, LockMode::FullCombined, first, 4});
  }
  return result;
}

std::optional<Scenario> scenario_by_label(const std::string& label) {
  for (const Scenario& scenario : all_scenarios()) if (scenario.label == label) return scenario;

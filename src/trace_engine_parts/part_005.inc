    }
    trace("LEGACY STAR", "R-RVS-01; R-VSTAR-01", "Discarded top 7: " + join_cards(discarded) + "; recovered: " + join_cards(recovered) + ".");
    return true;
  }

  bool use_celestial_roar() {
    if (first_turn_going_first() || state_.turn_ended || !state_.active || state_.active->card != Card::RegidragoV) return false;
    // R-RV-01: Celestial Roar costs one Colorless Energy. Any attached Energy pays it.
    if (state_.active->grass + state_.active->fire < 1) return false;
    // Attack policy: turn-one going-second Celestial Roar is used only when Steven's Resolve did not end the turn.
    if (state_.turn != 1 || scenario_.going_first) return false;
    std::vector<Card> discarded;
    for (int card = 0; card < 3 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      state_.discard.push_back(top);
      state_.discarded_this_turn.push_back(top);
      discarded.push_back(top);
      if (top == Card::Grass) ++state_.active->grass;
      if (top == Card::Fire) ++state_.active->fire;
    }
    state_.turn_ended = true;
    trace("ATTACK", "R-RV-01; R-GAME-ATTACK", "Regidrago V used Celestial Roar; discarded top 3: " + join_cards(discarded) + ". Attached all Basic Energy found to itself.");
    return true;
  }

  void play_basics_from_hand() {
    bool changed = true;
    while (changed && bench_space() > 0) {
      changed = false;
      if (need_regi() && hand_count(Card::RegidragoV) > 0) {
        changed = bench_from_hand(Card::RegidragoV, false);
      } else if (in_play_count(Card::RegidragoV) < 2 && hand_count(Card::RegidragoV) > 0 && !has_vstar()) {
        changed = bench_from_hand(Card::RegidragoV, false);
      } else if (need_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) > 0) {
        changed = bench_from_hand(Card::LatiasEx, false);
      } else if (bench_tapu_if_useful()) {
        changed = true;
      } else if (bench_oricorio_if_useful()) {
        changed = true;
      }
    }
  }

  bool run_search_items_one_step(const bool permit_payload) {
    if (item_locked()) return false;
    play_basics_from_hand();
    if (need_regi()) {
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
      if (play_heavy_ball()) return true;
    }
    if (need_vstar()) {
      if (play_evolution_incense()) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
    }
    if (need_active_vstar() && !in_play(Card::LatiasEx)) {
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
    }
    if (need_energy()) {
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
    if (!supporter_allowed()) return;
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
  return std::nullopt;
}

Aggregate simulate(const Scenario& scenario, const DeckRecipe& recipe, const std::uint64_t trials, const std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  Aggregate aggregate{};
  for (std::uint64_t trial = 0; trial < trials; ++trial) {
    Engine engine(scenario, recipe, rng);
    const TrialOutcome outcome = engine.run();
    ++aggregate.trials;
    aggregate.by2 += outcome.ready_by_2 ? 1U : 0U;
    aggregate.by3 += outcome.ready_by_3 ? 1U : 0U;
    aggregate.by4 += outcome.ready_by_4 ? 1U : 0U;
    aggregate.started_regi += outcome.started_regi ? 1U : 0U;
    aggregate.fss += outcome.used_fss ? 1U : 0U;
    aggregate.legacy += outcome.used_legacy_star ? 1U : 0U;
    aggregate.steven += outcome.used_steven ? 1U : 0U;
    aggregate.blender += outcome.used_blender ? 1U : 0U;
    aggregate.total_mulligans += outcome.mulligans;
  }
  return aggregate;
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(count) / static_cast<double>(total);
}

double standard_error_pp(const std::uint64_t count, const std::uint64_t total) {
  if (total == 0) return 0.0;
  const double p = static_cast<double>(count) / static_cast<double>(total);
  return 100.0 * std::sqrt(p * (1.0 - p) / static_cast<double>(total));
}

class OutputLock {
 public:
  explicit OutputLock(const std::filesystem::path& output) {
    const std::filesystem::path lock_path = output.string() + ".lock";
#if defined(_WIN32)
    handle_ = ::CreateFileW(lock_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) throw std::runtime_error("could not lock output");
#else
    descriptor_ = ::open(lock_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (descriptor_ < 0 || ::flock(descriptor_, LOCK_EX) != 0) {
      if (descriptor_ >= 0) ::close(descriptor_);
      throw std::runtime_error("could not lock output");
    }
#endif
  }
  ~OutputLock() {
#if defined(_WIN32)
    if (handle_ != INVALID_HANDLE_VALUE) ::CloseHandle(handle_);
#else
    if (descriptor_ >= 0) { ::flock(descriptor_, LOCK_UN); ::close(descriptor_); }
#endif
  }
  OutputLock(const OutputLock&) = delete;
  OutputLock& operator=(const OutputLock&) = delete;
 private:
#if defined(_WIN32)
  HANDLE handle_{INVALID_HANDLE_VALUE};
#else
  int descriptor_{-1};
#endif
};

void write_atomic(const std::filesystem::path& output, const std::string& content) {
  if (!output.parent_path().empty()) std::filesystem::create_directories(output.parent_path());
  OutputLock lock(output);
  const std::filesystem::path temporary = output.string() + ".tmp." + std::to_string(
#if defined(_WIN32)
      static_cast<unsigned long long>(::GetCurrentProcessId())
#else
      static_cast<unsigned long long>(::getpid())
#endif
  );
  {
    std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
    if (!stream) throw std::runtime_error("could not write temporary output");
    stream << content;
    stream.flush();
    if (!stream) throw std::runtime_error("could not flush temporary output");
  }
#if defined(_WIN32)
  if (!::MoveFileExW(temporary.c_str(), output.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("could not replace output");
#else
  std::error_code error;
  std::filesystem::rename(temporary, output, error);
  if (error) { std::filesystem::remove(temporary); throw std::runtime_error("could not replace output"); }
#endif
}

bool self_test() {
  const int total = std::accumulate(kDeckRecipe.begin(), kDeckRecipe.end(), 0, [](const int sum, const auto& entry) { return sum + entry.second; });
  if (total != 60) return false;
  if (!is_basic(Card::RegidragoV) || is_basic(Card::RegidragoVstar)) return false;
  if (!is_dragon_or_psychic(Card::RegidragoVstar) || !is_payload(Card::MegaDragonite) || is_payload(Card::Dipplin)) return false;

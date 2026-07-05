    discard_from_hand(*first);
    // Serena permits up to three discards. Extra cycling is limited to explicit matchup-flex fodder;
    // strict JIT retains protected cards after its one required discard.
    if (scenario_.dci != DciProfile::StrictJit) {
      for (int i = 0; i < 2; ++i) {
        const auto extra = choose_discard(false, true);
        if (!extra) break;
        if (*extra != Card::MawileGX && *extra != Card::Dipplin && *extra != Card::Guzma && *extra != Card::Channeler && *extra != Card::FieldBlower && *extra != Card::ChaoticSwell && *extra != Card::PathToPeak) break;
        discard_from_hand(*extra);
      }
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw();
    return true;
  }

  bool play_tate_draw() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || state_.hand.size() > 4U) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int i = 0; i < 5; ++i) draw();
    return true;
  }

  void bench_priority_basics() {
    bool progressed = true;
    while (progressed && bench_space() > 0) {
      progressed = false;
      if (!has_regi() && hand_count(Card::RegidragoV) > 0) {
        progressed = bench_basic(Card::RegidragoV, false);
      } else if (in_play_count(Card::RegidragoV) < 2 && hand_count(Card::RegidragoV) > 0) {
        progressed = bench_basic(Card::RegidragoV, false);
      } else if (needs_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) > 0) {
        progressed = bench_basic(Card::LatiasEx, false);
      } else if (!state_.supporter_used && !in_play(Card::TapuLeleGX) && hand_count(Card::TapuLeleGX) > 0) {
        progressed = bench_basic(Card::TapuLeleGX, true);
      } else if (need_energy() && !in_play(Card::Oricorio) && hand_count(Card::Oricorio) > 0) {
        progressed = bench_basic(Card::Oricorio, true);
      }
    }
  }

  void play_items(const bool allow_payload) {
    if (item_locked() || state_.turn_ended) return;
    for (int guard = 0; guard < 24; ++guard) {
      bench_priority_basics();
      if (play_heavy_ball()) continue;
      if (play_ultra_ball()) continue;
      if (play_evolution_incense()) continue;
      if (play_mysterious_treasure()) continue;
      if (play_quick_ball()) continue;
      if (play_earthen_vessel()) continue;
      if (allow_payload && play_blender()) continue;
      break;
    }
  }

  void choose_supporter() {
    if (!supporter_allowed()) return;
    if (play_tate_switch()) return;
    if (play_gladion()) return;
    if (state_.turn == 1 && !scenario_.going_first && has_regi_v() && hand_count(Card::StevensResolve) > 0 && play_steven()) return;
    if (need_energy() && hand_count(Card::Crispin) > 0 && play_crispin()) return;
    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && (item_locked() || hand_count(Card::BrilliantBlender) == 0) && play_burnet()) return;
    if (hand_count(Card::Arven) > 0 && play_arven()) return;
    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && play_burnet()) return;
    if (hand_count(Card::Serena) > 0 && play_serena()) return;
    play_tate_draw();
  }

  void run_turn() {
    bench_priority_basics();
    play_items(scenario_.dci == DciProfile::NoDiscardControl);
    bench_priority_basics();
    use_fss();
    bench_priority_basics();
    play_items(scenario_.dci == DciProfile::NoDiscardControl);

    attach_manual();
    choose_supporter();
    if (state_.turn_ended) return;

    bench_priority_basics();
    use_fss();
    bench_priority_basics();
    play_items(scenario_.dci == DciProfile::NoDiscardControl);
    attach_manual();
    evolve();
    bench_priority_basics();
    play_items(true);
    attach_manual();
    promote_vstar_with_latias();
  }

  void record_ready() {
    const bool ready = state_.turn >= 2 && active_is_vstar() && state_.active->grass >= 2 && state_.active->fire >= 1 && has_payload_in_discard();
    if (!ready) return;
    if (outcome_.first_ready_turn == 0) outcome_.first_ready_turn = state_.turn;
    if (state_.turn <= 2) outcome_.ready_by_2 = true;
    if (state_.turn <= 3) outcome_.ready_by_3 = true;
    if (state_.turn <= 4) outcome_.ready_by_4 = true;
  }
};

DeckRecipe baseline_recipe() { return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end()); }

bool adjust(DeckRecipe& recipe, const Card card, const int delta) {
  auto it = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) { return entry.first == card; });
  if (it == recipe.end()) {
    if (delta < 0) return false;
    recipe.push_back({card, delta});
    return true;
  }
  if (it->second + delta < 0) return false;
  it->second += delta;
  return true;
}

struct Variant { std::string label; DeckRecipe recipe; };

std::vector<Variant> variants() {
  const DeckRecipe baseline = baseline_recipe();
  std::vector<Variant> result{{"baseline", baseline}};
  const auto add = [&result, &baseline](std::string label, std::initializer_list<std::pair<Card, int>> edits) {
    DeckRecipe recipe = baseline;
    for (const auto& [card, delta] : edits) if (!adjust(recipe, card, delta)) throw std::runtime_error("invalid variant");
    const int total = std::accumulate(recipe.begin(), recipe.end(), 0, [](const int sum, const auto& entry) { return sum + entry.second; });
    if (total != 60) throw std::runtime_error("variant does not total 60");
    result.push_back({std::move(label), std::move(recipe)});
  };
  add("turbo/-Mawile+Crispin", {{Card::MawileGX, -1}, {Card::Crispin, 1}});
  add("turbo/-Mawile+Stevens", {{Card::MawileGX, -1}, {Card::StevensResolve, 1}});
  add("realism/-Mawile+Burnet", {{Card::MawileGX, -1}, {Card::ProfessorBurnet, 1}});
  add("turbo/-Mawile+EvolutionIncense", {{Card::MawileGX, -1}, {Card::EvolutionIncense, 1}});
  add("turbo/-Mawile,-Tate+QuickBall,+Crispin", {{Card::MawileGX, -1}, {Card::TateLiza, -1}, {Card::QuickBall, 1}, {Card::Crispin, 1}});
  return result;
}

std::vector<Scenario> scenarios() {
  std::vector<Scenario> result;
  for (const bool first : {true, false}) {
    const std::string seat = first ? "go-first" : "go-second";
    result.push_back({"strict-jit/" + seat, DciProfile::StrictJit, LockMode::None, first});
    result.push_back({"matchup-flex-jit/" + seat, DciProfile::MatchupFlexJit, LockMode::None, first});
    result.push_back({"no-discard-control/" + seat, DciProfile::NoDiscardControl, LockMode::None, first});
    result.push_back({"strict-jit-turn2-item-lock/" + seat, DciProfile::StrictJit, LockMode::TurnTwoItem, first});
    result.push_back({"strict-jit-full-item-lock/" + seat, DciProfile::StrictJit, LockMode::FullItem, first});
    result.push_back({"strict-jit-rulebox-ability-lock/" + seat, DciProfile::StrictJit, LockMode::FullRuleBoxAbility, first});
    result.push_back({"strict-jit-combined-lock/" + seat, DciProfile::StrictJit, LockMode::FullCombined, first});
  }
  return result;
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
    aggregate.opening_regidrago += outcome.opening_regidrago ? 1U : 0U;
    aggregate.fss += outcome.used_fss ? 1U : 0U;
    aggregate.steven += outcome.used_steven ? 1U : 0U;
    aggregate.blender += outcome.used_blender ? 1U : 0U;
    aggregate.mulligans += outcome.mulligans;
  }
  return aggregate;
}

double pct(const std::uint64_t count, const std::uint64_t total) { return total == 0 ? 0.0 : 100.0 * static_cast<double>(count) / static_cast<double>(total); }
double standard_error_pp(const std::uint64_t count, const std::uint64_t total) {
  if (total == 0) return 0.0;
  const double p = static_cast<double>(count) / static_cast<double>(total);
  return 100.0 * std::sqrt(p * (1.0 - p) / static_cast<double>(total));
}

std::string csv(std::string_view field) {
  std::string result{"\""};
  for (const char character : field) {
    if (character == '\"') result.push_back('\"');
    result.push_back(character);
  }
  result.push_back('\"');
  return result;
}

class OutputLock {
 public:
  explicit OutputLock(const std::filesystem::path& output) {
    const std::filesystem::path lock_path = output.string() + ".lock";

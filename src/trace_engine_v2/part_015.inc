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
    aggregate.by5 += outcome.ready_by_5 ? 1U : 0U;
    aggregate.ready_on5 += outcome.first_ready_turn == 5 ? 1U : 0U;
    aggregate.setup_failures += outcome.setup_failed ? 1U : 0U;
    aggregate.started_regi += outcome.started_regi ? 1U : 0U;
    aggregate.started_tapu += outcome.started_tapu ? 1U : 0U;
    aggregate.fss += outcome.used_fss ? 1U : 0U;
    aggregate.legacy += outcome.used_legacy_star ? 1U : 0U;
    aggregate.steven += outcome.used_steven ? 1U : 0U;
    aggregate.blender += outcome.used_blender ? 1U : 0U;
    aggregate.secret_box += outcome.used_secret_box ? 1U : 0U;
    aggregate.exploding_energy += outcome.used_exploding_energy ? 1U : 0U;
    aggregate.secret_box_combo_attempted += outcome.secret_box_combo_attempted;
    aggregate.secret_box_cost_blocked += outcome.secret_box_cost_blocked;
    aggregate.secret_box_missing_axis += outcome.secret_box_missing_axis;
    aggregate.secret_box_missing_regi_axis += outcome.secret_box_missing_regi_axis;
    aggregate.secret_box_missing_line_axis += outcome.secret_box_missing_line_axis;
    aggregate.secret_box_missing_vstar_axis += outcome.secret_box_missing_vstar_axis;
    aggregate.secret_box_missing_payload_axis += outcome.secret_box_missing_payload_axis;
    aggregate.secret_box_missing_treasure_axis += outcome.secret_box_missing_treasure_axis;
    aggregate.secret_box_missing_fire_axis += outcome.secret_box_missing_fire_axis;
    aggregate.secret_box_missing_grass_axis += outcome.secret_box_missing_grass_axis;
    aggregate.secret_box_missing_ability_axis += outcome.secret_box_missing_ability_axis;
    aggregate.secret_box_missing_supporter_axis += outcome.secret_box_missing_supporter_axis;
    aggregate.secret_box_missing_known_prize_zone += outcome.secret_box_missing_known_prize_zone;
    aggregate.secret_box_missing_discard_zone += outcome.secret_box_missing_discard_zone;
    aggregate.secret_box_missing_stranded_hand_zone += outcome.secret_box_missing_stranded_hand_zone;
    aggregate.secret_box_bench_blocked += outcome.secret_box_bench_blocked;
    aggregate.secret_box_arven_bank += outcome.secret_box_arven_bank;
    aggregate.secret_box_steven_bank += outcome.secret_box_steven_bank;
    aggregate.secret_box_gladion_bank += outcome.secret_box_gladion_bank;
    aggregate.secret_box_fss_bank += outcome.secret_box_fss_bank;
    aggregate.t2_missing_active_vstar += outcome.t2_missing_active_vstar ? 1U : 0U;
    aggregate.t2_missing_grass += outcome.t2_missing_grass ? 1U : 0U;
    aggregate.t2_missing_fire += outcome.t2_missing_fire ? 1U : 0U;
    aggregate.t2_missing_payload += outcome.t2_missing_payload ? 1U : 0U;
    aggregate.t2_eligible_regi_start += outcome.t2_eligible_regi_start ? 1U : 0U;
    aggregate.t2_active_regi_start += outcome.t2_active_regi_start ? 1U : 0U;
    aggregate.t2_pineco_line_start += outcome.t2_pineco_line_start ? 1U : 0U;
    aggregate.t2_vstar_held_start += outcome.t2_vstar_held_start ? 1U : 0U;
    aggregate.t2_eligible_held_vstar_missed_active +=
        outcome.t2_eligible_held_vstar_missed_active ? 1U : 0U;
    aggregate.t2_eligible_missed_active +=
        outcome.t2_eligible_missed_active ? 1U : 0U;
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

bool probability_regression_self_test() {
  struct BaselineFloor {
    std::string_view scenario;
    double minimum_ready_by_t3_pct;
  };

  // These are deliberately conservative floors beneath the generated cumulative
  // T3 baseline, so the guard catches an order-of-magnitude collapse without
  // treating normal policy improvements as a failure:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/REPORT.md#100000-trial-baseline
  constexpr std::array<BaselineFloor, 3> floors{{
      {"strict-jit/go-second", 20.0},
      {"matchup-flex-jit/go-second", 30.0},
      {"no-discard-control/go-second", 40.0},
  }};
  constexpr std::uint64_t trials = 2000;
  constexpr std::uint64_t base_seed = 20260710;
  constexpr std::uint64_t seed_stride = 104729;
  const DeckRecipe deck = baseline_recipe();
  const std::vector<Scenario> scenarios = all_scenarios();

  for (const BaselineFloor& floor : floors) {
    const auto found = std::find_if(
        scenarios.begin(), scenarios.end(), [&floor](const Scenario& scenario) {
          return scenario.label == floor.scenario;
        });
    if (found == scenarios.end()) return false;
    const std::size_t scenario_index = static_cast<std::size_t>(
        std::distance(scenarios.begin(), found));
    const Aggregate aggregate = simulate(*found, deck, trials,
                                         base_seed + seed_stride * scenario_index);
    // Readiness columns are cumulative. T5 is diagnostic; setup success still ends at T4.
    if (aggregate.by3 < aggregate.by2 || aggregate.by4 < aggregate.by3 ||
        aggregate.by5 < aggregate.by4) return false;
    if (aggregate.ready_on5 != aggregate.by5 - aggregate.by4) return false;
    if (aggregate.setup_failures != aggregate.trials - aggregate.by4) return false;
    if (pct(aggregate.by3, aggregate.trials) < floor.minimum_ready_by_t3_pct) return false;
  }
  return true;
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
  if (!probability_regression_self_test()) return false;
  if (deck_registry().size() != 2U) return false;
  for (const NamedDeck& deck : deck_registry()) {
    std::string error;
    if (!validate_recipe(deck, &error)) return false;
  }
  if (deck_by_id("regidrago-shell") == nullptr ||
      deck_by_id("regidrago-pineco") == nullptr ||
      deck_by_id("unknown") != nullptr) return false;
  if (!is_basic(Card::RegidragoV) || is_basic(Card::RegidragoVstar) ||
      is_basic(Card::Appletun)) return false;
  if (!is_dragon_or_psychic(Card::RegidragoVstar) ||
      !is_payload(Card::MegaDragonite) || !is_payload(Card::Appletun) ||
      is_payload(Card::Dipplin)) return false;
  if (!is_ace_spec(Card::SecretBox) || !is_item(Card::SecretBox)) return false;
  if (is_rule_box_pokemon(Card::Oricorio)) return false;
  if (!is_rule_box_pokemon(Card::TapuLeleGX)) return false;
  // Pokémon V includes Pokémon VSTAR: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  if (!is_pokemon_v(Card::RegidragoV) || !is_pokemon_v(Card::RegidragoVstar)) return false;
  // Trace smoke validates a complete single-game setup and rule ID output path.
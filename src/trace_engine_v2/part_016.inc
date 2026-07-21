  const auto scenario = scenario_by_label("strict-jit/go-second");
  if (!scenario) return false;
  const DeckRecipe deck = baseline_recipe();
  // Reuse the registered strict-JIT go-second T3 regression seed and deadline:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt
  std::mt19937_64 rng(1);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck, rng, &trace);
  const TrialOutcome outcome = engine.run();
  if (trace.lines.empty()) return false;
  const bool has_ready_rule_trace = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("READY | rules: R-RVS-01; P-JIT-01") != std::string::npos;
      });
  return outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 3 && has_ready_rule_trace;
}

}  // namespace sim

#ifndef REGIDRAGO_SIM_NO_MAIN
std::optional<std::uint64_t> parse_unsigned_decimal(const std::string& text) {
  const bool decimal_digits_only = !text.empty() && std::all_of(
      text.begin(), text.end(), [](const unsigned char character) {
        return character >= static_cast<unsigned char>('0') &&
               character <= static_cast<unsigned char>('9');
      });
  if (!decimal_digits_only) return std::nullopt;
  try {
    return std::stoull(text);
  } catch (const std::out_of_range&) {
    return std::nullopt;
  }
}

std::optional<std::uint64_t> parse_positive_count(const std::string& text) {
  const auto parsed = parse_unsigned_decimal(text);
  if (!parsed || *parsed == 0) return std::nullopt;
  return parsed;
}

std::optional<int> parse_ready_deadline(const std::string& text) {
  const auto parsed = parse_positive_count(text);
  if (!parsed || *parsed > 5) return std::nullopt;
  return static_cast<int>(*parsed);
}

enum class CliMode : std::uint8_t {
  Aggregate,
  SingleTrace,
  FindReady,
  SelfTest,
};

int main(int argc, char** argv) {
  std::uint64_t trials = 100000;
  std::uint64_t seed = 20260705;
  std::string output = "results/simulation_results.csv";
  std::string scenario_label;
  std::string deck_id = "regidrago-shell";
  bool all_decks = false;
  bool self_test = false;
  bool simulate_this = false;
  int require_ready_by = 0;
  std::uint64_t find_ready = 0;
  std::uint64_t start_seed = 1;

  bool trials_supplied = false;
  bool seed_supplied = false;
  bool output_supplied = false;
  bool scenario_supplied = false;
  bool deck_supplied = false;
  bool deadline_supplied = false;
  bool find_ready_supplied = false;
  bool start_seed_supplied = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--trials" && i + 1 < argc) {
      const auto parsed = parse_positive_count(argv[++i]);
      if (!parsed) {
        std::cerr << "--trials must be a positive integer\n";
        return 2;
      }
      trials = *parsed;
      trials_supplied = true;
    } else if (arg == "--seed" && i + 1 < argc) {
      const auto parsed = parse_unsigned_decimal(argv[++i]);
      if (!parsed) {
        std::cerr << "--seed must be an unsigned integer\n";
        return 2;
      }
      seed = *parsed;
      seed_supplied = true;
    } else if (arg == "--out" && i + 1 < argc) {
      output = argv[++i];
      output_supplied = true;
    } else if (arg == "--scenario" && i + 1 < argc) {
      scenario_label = argv[++i];
      scenario_supplied = true;
    } else if (arg == "--deck" && i + 1 < argc) {
      deck_id = argv[++i];
      deck_supplied = true;
    } else if (arg == "--all-decks") {
      all_decks = true;
    } else if (arg == "--simulate-this") {
      simulate_this = true;
    } else if (arg == "--require-ready-by" && i + 1 < argc) {
      const auto parsed = parse_ready_deadline(argv[++i]);
      if (!parsed) {
        std::cerr << "--require-ready-by must be an integer from 1 through 5\n";
        return 2;
      }
      require_ready_by = *parsed;
      deadline_supplied = true;
    } else if (arg == "--find-ready" && i + 1 < argc) {
      const auto parsed = parse_positive_count(argv[++i]);
      if (!parsed) {
        std::cerr << "--find-ready must be a positive integer\n";
        return 2;
      }
      find_ready = *parsed;
      find_ready_supplied = true;
    } else if (arg == "--start-seed" && i + 1 < argc) {
      const auto parsed = parse_unsigned_decimal(argv[++i]);
      if (!parsed) {
        std::cerr << "--start-seed must be an unsigned integer\n";
        return 2;
      }
      start_seed = *parsed;
      start_seed_supplied = true;
    } else if (arg == "--self-test") {
      self_test = true;
    } else if (arg == "--help") {
      std::cout
          << "Usage: regidrago_sim [--deck ID | --all-decks] [--trials N] "
             "[--seed N] [--out PATH] [--scenario LABEL] [--simulate-this] "
             "[--require-ready-by TURN] [--find-ready N --start-seed N] "
             "[--self-test]\n\nRegistered decks:\n";
      for (const sim::NamedDeck& deck : sim::deck_registry()) {
        std::cout << "  " << deck.id << '\n';
      }
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << '\n';
      return 2;
    }
  }

  const int explicit_mode_count = static_cast<int>(self_test) +
      static_cast<int>(simulate_this) + static_cast<int>(find_ready_supplied);
  if (explicit_mode_count > 1) {
    std::cerr << "--self-test, --simulate-this, and --find-ready select incompatible modes\n";
    return 2;
  }

  const CliMode mode = self_test ? CliMode::SelfTest :
      (find_ready_supplied ? CliMode::FindReady :
       (simulate_this ? CliMode::SingleTrace : CliMode::Aggregate));
  const auto reject_option = [](const char* option, const char* mode_name) {
    std::cerr << option << " is not valid in " << mode_name << " mode\n";
    return 2;
  };

  if (deck_supplied && all_decks) {
    std::cerr << "--deck and --all-decks are mutually exclusive\n";
    return 2;
  }
  if (mode != CliMode::Aggregate && all_decks) {
    return reject_option("--all-decks", "trace");
  }
  if (mode == CliMode::Aggregate) {
    if (scenario_supplied) return reject_option("--scenario", "aggregate");
    if (deadline_supplied) return reject_option("--require-ready-by", "aggregate");
    if (start_seed_supplied) return reject_option("--start-seed", "aggregate");
  } else if (mode == CliMode::SingleTrace) {
    if (trials_supplied) return reject_option("--trials", "single-trace");
    if (output_supplied) return reject_option("--out", "single-trace");
    if (start_seed_supplied) return reject_option("--start-seed", "single-trace");
  } else if (mode == CliMode::FindReady) {
    if (trials_supplied) return reject_option("--trials", "find-ready");
    if (output_supplied) return reject_option("--out", "find-ready");
    if (seed_supplied) return reject_option("--seed", "find-ready");
    if (deadline_supplied && require_ready_by == 1) {
      std::cerr << "--require-ready-by 1 cannot be satisfied in find-ready mode; readiness begins on turn 2\n";
      return 2;
    }
  } else {
    if (trials_supplied) return reject_option("--trials", "self-test");
    if (seed_supplied) return reject_option("--seed", "self-test");
    if (output_supplied) return reject_option("--out", "self-test");
    if (scenario_supplied) return reject_option("--scenario", "self-test");
    if (deck_supplied) return reject_option("--deck", "self-test");
    if (all_decks) return reject_option("--all-decks", "self-test");
    if (deadline_supplied) return reject_option("--require-ready-by", "self-test");
    if (start_seed_supplied) return reject_option("--start-seed", "self-test");
  }

  if (mode == CliMode::SelfTest) {
    const auto one_count = parse_positive_count("1");
    const bool positive_count_parser_ok = one_count && *one_count == 1 &&
        !parse_positive_count("0") && !parse_positive_count("-1") &&
        !parse_positive_count("") &&
        !parse_positive_count("18446744073709551616");
    const auto turn_one = parse_ready_deadline("1");
    const auto turn_five = parse_ready_deadline("5");
    const bool deadline_parser_ok = turn_one && *turn_one == 1 &&
        turn_five && *turn_five == 5 && !parse_ready_deadline("0") &&
        !parse_ready_deadline("-1") && !parse_ready_deadline("+1") &&
        !parse_ready_deadline("6") && !parse_ready_deadline("") &&
        !parse_ready_deadline("18446744073709551616");
    const auto zero_seed = parse_unsigned_decimal("0");
    const auto ordinary_seed = parse_unsigned_decimal("20260705");
    const auto maximum_seed = parse_unsigned_decimal("18446744073709551615");
    const bool seed_parser_ok = zero_seed && *zero_seed == 0 &&
        ordinary_seed && *ordinary_seed == 20260705ULL &&
        maximum_seed && *maximum_seed == ~std::uint64_t{0} &&
        !parse_unsigned_decimal("-1") && !parse_unsigned_decimal("+1") &&
        !parse_unsigned_decimal("nope") && !parse_unsigned_decimal(" 1") &&
        !parse_unsigned_decimal("") &&
        !parse_unsigned_decimal("18446744073709551616");
    return sim::self_test() && positive_count_parser_ok && deadline_parser_ok &&
        seed_parser_ok ? 0 : 1;
  }

  try {
    const sim::NamedDeck* selected_deck = sim::deck_by_id(deck_id);
    if (selected_deck == nullptr) {
      throw std::runtime_error("unknown deck: " + deck_id);
    }

    if (mode == CliMode::SingleTrace || mode == CliMode::FindReady) {
      const std::string selected_scenario =
          scenario_label.empty() ? "strict-jit/go-second" : scenario_label;
      const auto scenario = sim::scenario_by_label(selected_scenario);
      if (!scenario) throw std::runtime_error("unknown scenario: " + selected_scenario);
      const std::uint64_t desired = mode == CliMode::SingleTrace ? 1 : find_ready;
      std::uint64_t found = 0;
      std::uint64_t candidate_seed =
          mode == CliMode::SingleTrace ? seed : start_seed;
      while (found < desired) {
        std::mt19937_64 rng(candidate_seed);
        sim::TraceLog trace{true, {}};
        sim::Engine engine(*scenario, selected_deck->recipe, rng, &trace);
        const sim::TrialOutcome outcome = engine.run();
        const bool misses_required_deadline = require_ready_by > 0 &&
            (outcome.first_ready_turn == 0 ||
             outcome.first_ready_turn > require_ready_by);
        const bool find_ready_miss = mode == CliMode::FindReady &&
            (outcome.first_ready_turn == 0 || misses_required_deadline);
        if (!find_ready_miss) {
          const std::string ready_summary = outcome.first_ready_turn == 0
              ? "not ready through T5"
              : std::to_string(outcome.first_ready_turn);
          const std::string setup_result =
              outcome.setup_failed ? "failure" : "success";
          std::cout << "\nDeck: " << selected_deck->id << " | Seed: "
                    << candidate_seed << " | first-ready turn: "
                    << ready_summary << " | setup result: "
                    << setup_result << '\n';
          // Preserve the historical seed-only summary for downstream trace checks.
          std::cout << "Seed: " << candidate_seed << " | first-ready turn: "
                    << ready_summary << " | setup result: "
                    << setup_result << '\n';
          for (const std::string& line : trace.lines) std::cout << line << '\n';
          if (misses_required_deadline) {
            std::cerr << "Trace did not reach a ready state by required turn "
                      << require_ready_by << "\n";
            return 1;
          }
          ++found;
          if (mode == CliMode::SingleTrace) break;
        }
        if (found >= desired) break;
        if (candidate_seed == ~std::uint64_t{0}) {
          throw std::runtime_error(
              "find-ready search exhausted the uint64 seed range");
        }
        ++candidate_seed;
      }
      return 0;
    }

    std::vector<const sim::NamedDeck*> decks;
    if (all_decks) {
      for (const sim::NamedDeck& deck : sim::deck_registry()) decks.push_back(&deck);
    } else {
      decks.push_back(selected_deck);
    }

    std::ostringstream csv;
    csv << "deck,scenario,trials,ready_by_t2_pct,ready_by_t2_se_pp,"
           "ready_by_t3_pct,ready_by_t3_se_pp,ready_by_t4_pct,"
           "ready_by_t4_se_pp,ready_by_t5_pct,ready_by_t5_se_pp,"
           "ready_on_t2_pct,ready_on_t3_pct,ready_on_t4_pct,ready_on_t5_pct,"
           "setup_failure_pct,setup_failure_se_pp,started_regi_pct,"
           "started_tapu_pct,secret_box_used_pct,blender_used_pct,"
           "exploding_energy_used_pct,steven_used_pct,star_alchemy_used_pct,"
           "legacy_star_used_pct,mean_mulligans,secret_box_attempts_per_game,"
           "secret_box_cost_blocks_per_game,secret_box_missing_axis_per_game,"
           "secret_box_missing_regi_axis_per_game,"
           "secret_box_missing_line_axis_per_game,"
           "secret_box_missing_vstar_axis_per_game,"
           "secret_box_missing_payload_axis_per_game,"
           "secret_box_missing_treasure_axis_per_game,"
           "secret_box_missing_fire_axis_per_game,"
           "secret_box_missing_grass_axis_per_game,"
           "secret_box_missing_ability_axis_per_game,"
           "secret_box_missing_supporter_axis_per_game,"
           "secret_box_missing_known_prize_zone_per_game,"
           "secret_box_missing_discard_zone_per_game,"
           "secret_box_missing_stranded_hand_zone_per_game,"
           "secret_box_bench_blocks_per_game,secret_box_arven_banks_per_game,"
           "secret_box_steven_banks_per_game,secret_box_gladion_banks_per_game,"
           "secret_box_fss_banks_per_game,t2_missing_active_vstar_pct,"
           "t2_missing_grass_pct,t2_missing_fire_pct,t2_missing_payload_pct,"
           "t2_eligible_regi_start_pct,t2_active_regi_start_pct,"
           "t2_pineco_line_start_pct,t2_vstar_held_start_pct,"
           "t2_eligible_missed_active_pct,"
           "t2_eligible_held_vstar_missed_active_pct\n";
    const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
    for (const sim::NamedDeck* deck : decks) {
      for (std::size_t index = 0; index < scenarios.size(); ++index) {
        const sim::Scenario& scenario = scenarios[index];
        // Decks share the same scenario seed, creating common random numbers for
        // paired comparisons while preserving the historical shell seed schedule:
        // https://github.com/FlareZ123/pokemon-sims/issues/1118
        const sim::Aggregate a = sim::simulate(
            scenario, deck->recipe, trials, seed + 104729ULL * index);
        const auto per_game = [&a](const std::uint64_t count) {
          return a.trials == 0 ? 0.0 :
              static_cast<double>(count) / static_cast<double>(a.trials);
        };
        csv << '"' << deck->id << "\",\"" << scenario.label << "\","
            << a.trials << ','
            << sim::pct(a.by2, a.trials) << ','
            << sim::standard_error_pp(a.by2, a.trials) << ','
            << sim::pct(a.by3, a.trials) << ','
            << sim::standard_error_pp(a.by3, a.trials) << ','
            << sim::pct(a.by4, a.trials) << ','
            << sim::standard_error_pp(a.by4, a.trials) << ','
            << sim::pct(a.by5, a.trials) << ','
            << sim::standard_error_pp(a.by5, a.trials) << ','
            << sim::pct(a.by2, a.trials) << ','
            << sim::pct(a.by3 - a.by2, a.trials) << ','
            << sim::pct(a.by4 - a.by3, a.trials) << ','
            << sim::pct(a.ready_on5, a.trials) << ','
            << sim::pct(a.setup_failures, a.trials) << ','
            << sim::standard_error_pp(a.setup_failures, a.trials) << ','
            << sim::pct(a.started_regi, a.trials) << ','
            << sim::pct(a.started_tapu, a.trials) << ','
            << sim::pct(a.secret_box, a.trials) << ','
            << sim::pct(a.blender, a.trials) << ','
            << sim::pct(a.exploding_energy, a.trials) << ','
            << sim::pct(a.steven, a.trials) << ','
            << sim::pct(a.fss, a.trials) << ','
            << sim::pct(a.legacy, a.trials) << ','
            << per_game(static_cast<std::uint64_t>(a.total_mulligans)) << ','
            << per_game(a.secret_box_combo_attempted) << ','
            << per_game(a.secret_box_cost_blocked) << ','
            << per_game(a.secret_box_missing_axis) << ','
            << per_game(a.secret_box_missing_regi_axis) << ','
            << per_game(a.secret_box_missing_line_axis) << ','
            << per_game(a.secret_box_missing_vstar_axis) << ','
            << per_game(a.secret_box_missing_payload_axis) << ','
            << per_game(a.secret_box_missing_treasure_axis) << ','
            << per_game(a.secret_box_missing_fire_axis) << ','
            << per_game(a.secret_box_missing_grass_axis) << ','
            << per_game(a.secret_box_missing_ability_axis) << ','
            << per_game(a.secret_box_missing_supporter_axis) << ','
            << per_game(a.secret_box_missing_known_prize_zone) << ','
            << per_game(a.secret_box_missing_discard_zone) << ','
            << per_game(a.secret_box_missing_stranded_hand_zone) << ','
            << per_game(a.secret_box_bench_blocked) << ','
            << per_game(a.secret_box_arven_bank) << ','
            << per_game(a.secret_box_steven_bank) << ','
            << per_game(a.secret_box_gladion_bank) << ','
            << per_game(a.secret_box_fss_bank) << ','
            << sim::pct(a.t2_missing_active_vstar, a.trials) << ','
            << sim::pct(a.t2_missing_grass, a.trials) << ','
            << sim::pct(a.t2_missing_fire, a.trials) << ','
            << sim::pct(a.t2_missing_payload, a.trials) << ','
            << sim::pct(a.t2_eligible_regi_start, a.trials) << ','
            << sim::pct(a.t2_active_regi_start, a.trials) << ','
            << sim::pct(a.t2_pineco_line_start, a.trials) << ','
            << sim::pct(a.t2_vstar_held_start, a.trials) << ','
            << sim::pct(a.t2_eligible_missed_active, a.trials) << ','
            << sim::pct(a.t2_eligible_held_vstar_missed_active, a.trials)
            << '\n';
      }
    }
    sim::write_atomic(output, csv.str());
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
#endif  // REGIDRAGO_SIM_NO_MAIN

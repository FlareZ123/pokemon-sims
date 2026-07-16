  const auto scenario = scenario_by_label("strict-jit/go-second");
  if (!scenario) return false;
  const DeckRecipe deck = baseline_recipe();
  // Reuse the registered strict-JIT go-second T3 regression seed and deadline:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt
  std::mt19937_64 rng(6);
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
std::optional<std::uint64_t> parse_positive_count(const std::string& text) {
  // Aggregate trials and requested ready traces are positive sample counts:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
  // std::stoull accepts an optional sign, so validate the decimal spelling first:
  // https://en.cppreference.com/w/cpp/string/basic_string/stoul
  const bool decimal_digits_only = !text.empty() && std::all_of(
      text.begin(), text.end(), [](const unsigned char character) {
        return character >= static_cast<unsigned char>('0') &&
               character <= static_cast<unsigned char>('9');
      });
  if (!decimal_digits_only) return std::nullopt;

  try {
    const std::uint64_t value = std::stoull(text);
    if (value == 0) return std::nullopt;
    return value;
  } catch (const std::out_of_range&) {
    return std::nullopt;
  }
}

std::optional<int> parse_ready_deadline(const std::string& text) {
  const auto parsed = parse_positive_count(text);
  // Trace scenarios model readiness only through turn 4. A deadline must select one
  // of those positive modeled turns instead of becoming a sentinel that disables
  // the later check:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L33
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
  if (!parsed || *parsed > 4) return std::nullopt;
  return static_cast<int>(*parsed);
}

int main(int argc, char** argv) {
  std::uint64_t trials = 100000;
  std::uint64_t seed = 20260705;
  std::string output = "results/simulation_results.csv";
  std::string scenario_label;
  bool self_test = false;
  bool simulate_this = false;
  int require_ready_by = 0;
  std::uint64_t find_ready = 0;
  std::uint64_t start_seed = 1;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--trials" && i + 1 < argc) {
      const auto parsed_trials = parse_positive_count(argv[++i]);
      if (!parsed_trials) {
        std::cerr << "--trials must be a positive integer\n";
        return 2;
      }
      trials = *parsed_trials;
    } else if (arg == "--seed" && i + 1 < argc) seed = std::stoull(argv[++i]);
    else if (arg == "--out" && i + 1 < argc) output = argv[++i];
    else if (arg == "--scenario" && i + 1 < argc) scenario_label = argv[++i];
    else if (arg == "--simulate-this") simulate_this = true;
    else if (arg == "--require-ready-by" && i + 1 < argc) {
      const auto parsed_deadline = parse_ready_deadline(argv[++i]);
      if (!parsed_deadline) {
        std::cerr << "--require-ready-by must be an integer from 1 through 4\n";
        return 2;
      }
      require_ready_by = *parsed_deadline;
    } else if (arg == "--find-ready" && i + 1 < argc) {
      // --find-ready is a requested result count. Reject signed, zero, and
      // out-of-range spellings before entering the trace-generation loop:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
      // https://en.cppreference.com/w/cpp/string/basic_string/stoul
      const auto parsed_find_ready = parse_positive_count(argv[++i]);
      if (!parsed_find_ready) {
        std::cerr << "--find-ready must be a positive integer\n";
        return 2;
      }
      find_ready = *parsed_find_ready;
    } else if (arg == "--start-seed" && i + 1 < argc) start_seed = std::stoull(argv[++i]);
    else if (arg == "--self-test") self_test = true;
    else if (arg == "--help") {
      std::cout << "Usage: regidrago_sim [--trials N] [--seed N] [--out PATH] [--scenario LABEL] [--simulate-this] [--require-ready-by TURN] [--find-ready N --start-seed N] [--self-test]\n";
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << '\n';
      return 2;
    }
  }

  if (self_test) {
    // Keep both documented positive-count CLI contracts in the existing self-test:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
    const auto one_count = parse_positive_count("1");
    const bool positive_count_parser_ok = one_count && *one_count == 1 &&
        !parse_positive_count("0") && !parse_positive_count("-1") &&
        !parse_positive_count("") &&
        !parse_positive_count("18446744073709551616");
    // Exercise both accepted modeled deadlines and rejected nonpositive, signed,
    // out-of-horizon, empty, and overflow operands through the CLI parser:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L33
    // https://github.com/FlareZ123/pokemon-sims/issues/626
    const auto turn_one = parse_ready_deadline("1");
    const auto turn_four = parse_ready_deadline("4");
    const bool deadline_parser_ok = turn_one && *turn_one == 1 &&
        turn_four && *turn_four == 4 && !parse_ready_deadline("0") &&
        !parse_ready_deadline("-1") && !parse_ready_deadline("+1") &&
        !parse_ready_deadline("5") && !parse_ready_deadline("") &&
        !parse_ready_deadline("18446744073709551616");
    return sim::self_test() && positive_count_parser_ok && deadline_parser_ok ? 0 : 1;
  }

  try {
    const sim::DeckRecipe deck = sim::baseline_recipe();
    if (simulate_this || find_ready > 0) {
      const std::string selected = scenario_label.empty() ? "strict-jit/go-second" : scenario_label;
      const auto scenario = sim::scenario_by_label(selected);
      if (!scenario) throw std::runtime_error("unknown scenario: " + selected);
      const std::uint64_t desired = find_ready == 0 ? 1 : find_ready;
      std::uint64_t found = 0;
      for (std::uint64_t candidate_seed = find_ready == 0 ? seed : start_seed; found < desired; ++candidate_seed) {
        std::mt19937_64 rng(candidate_seed);
        sim::TraceLog trace{true, {}};
        sim::Engine engine(*scenario, deck, rng, &trace);
        const sim::TrialOutcome outcome = engine.run();
        if (find_ready > 0 && outcome.first_ready_turn == 0) continue;
        std::cout << "\nSeed: " << candidate_seed << " | first-ready turn: " << (outcome.first_ready_turn == 0 ? std::string("not ready through T4") : std::to_string(outcome.first_ready_turn)) << '\n';
        for (const std::string& line : trace.lines) std::cout << line << '\n';
        if (require_ready_by > 0 && (outcome.first_ready_turn == 0 || outcome.first_ready_turn > require_ready_by)) {
          std::cerr << "Trace did not reach a ready state by required turn " << require_ready_by << "\n";
          return 1;
        }
        ++found;
        if (find_ready == 0) break;
      }
      return 0;
    }

    std::ostringstream csv;
    csv << "scenario,trials,ready_by_t2_pct,ready_by_t2_se_pp,ready_by_t3_pct,ready_by_t3_se_pp,ready_by_t4_pct,ready_by_t4_se_pp,started_regi_pct,fss_used_pct,legacy_star_used_pct,steven_used_pct,blender_used_pct,mean_mulligans\n";
    const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
    for (std::size_t index = 0; index < scenarios.size(); ++index) {
      const sim::Scenario& scenario = scenarios[index];
      const sim::Aggregate a = sim::simulate(scenario, deck, trials, seed + 104729ULL * index);
      csv << '"' << scenario.label << "\"," << a.trials << ',' << sim::pct(a.by2, a.trials) << ',' << sim::standard_error_pp(a.by2, a.trials) << ','
          << sim::pct(a.by3, a.trials) << ',' << sim::standard_error_pp(a.by3, a.trials) << ',' << sim::pct(a.by4, a.trials) << ','
          << sim::standard_error_pp(a.by4, a.trials) << ',' << sim::pct(a.started_regi, a.trials) << ',' << sim::pct(a.fss, a.trials) << ','
          << sim::pct(a.legacy, a.trials) << ',' << sim::pct(a.steven, a.trials) << ',' << sim::pct(a.blender, a.trials) << ','
          << (a.total_mulligans / static_cast<double>(a.trials)) << '\n';
    }
    sim::write_atomic(output, csv.str());
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
#endif  // REGIDRAGO_SIM_NO_MAIN

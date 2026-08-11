// Shared Professor's Letter experiment driver.
// Confirmed overcomputation bug and one-pass evidence contract:
// https://github.com/FlareZ123/pokemon-sims/issues/3008
// Existing real-game analysis reused below:
// https://github.com/FlareZ123/pokemon-sims/blob/main/tools/professors_letter_real_game_analysis.cpp

#define main professors_letter_real_game_analysis_legacy_main
#include "professors_letter_real_game_analysis.cpp"
#undef main

namespace sim {

struct SetupCheckpoint {
  bool regi{false};
  bool vstar{false};
  bool active_vstar{false};
  bool energy{false};
  bool payload{false};
  bool k1{false};
};

struct TrialWithCheckpoints {
  TrialOutcome outcome;
  std::array<SetupCheckpoint, 5> by_turn{};
};

// Engine intentionally friends this test-access shim. Keep checkpoint collection
// on the exact production turn loop so the merged experiment observes the same
// state as Engine::run(), while exposing no new production API just for evidence.
// Engine source/spec: https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp
// Confirmed one-pass requirement: https://github.com/FlareZ123/pokemon-sims/issues/3008
struct EngineTestAccess {
  static SetupCheckpoint checkpoint(Engine& engine) {
    const Pokemon* target = engine.target_regi();
    return {
        engine.in_play(Card::RegidragoV) ||
            engine.in_play(Card::RegidragoVstar),
        engine.in_play(Card::RegidragoVstar),
        engine.active_is_vstar(),
        target != nullptr && engine.pays_apex_energy_cost(*target),
        engine.payload_ready(),
        engine.prizes_known(),
    };
  }

  static TrialWithCheckpoints run(Engine& engine) {
    TrialWithCheckpoints result;
    engine.setup();
    SetupCheckpoint latest;
    for (int next_turn = 1; next_turn <= engine.scenario_.max_turn; ++next_turn) {
      engine.begin_turn(next_turn);
      if (engine.state_.turn_ended) break;
      engine.run_turn();
      engine.record_ready();
      latest = checkpoint(engine);
      result.by_turn[static_cast<std::size_t>(next_turn - 1)] = latest;
      if (engine.outcome_.first_ready_turn != 0) {
        for (int future = next_turn + 1; future <= 5; ++future) {
          result.by_turn[static_cast<std::size_t>(future - 1)] = latest;
        }
        break;
      }
      engine.resolve_powerglass_end_turn();
    }
    result.outcome = engine.outcome_;
    return result;
  }
};

TrialWithCheckpoints run_game_with_checkpoints(
    const Scenario& scenario, const DeckRecipe& recipe,
    const std::uint64_t seed, TraceLog* trace = nullptr) {
  std::mt19937_64 rng(seed);
  Engine engine(scenario, recipe, rng, trace);
  return EngineTestAccess::run(engine);
}

struct CheckpointAggregate {
  std::uint64_t trials{0};
  std::uint64_t by2{0};
  std::uint64_t by3{0};
  std::uint64_t by4{0};
  std::array<std::uint64_t, 5> regi{};
  std::array<std::uint64_t, 5> vstar{};
  std::array<std::uint64_t, 5> active_vstar{};
  std::array<std::uint64_t, 5> energy{};
  std::array<std::uint64_t, 5> payload{};
  std::array<std::uint64_t, 5> k1{};

  void add(const TrialWithCheckpoints& trial) {
    ++trials;
    by2 += trial.outcome.ready_by_2 ? 1U : 0U;
    by3 += trial.outcome.ready_by_3 ? 1U : 0U;
    by4 += trial.outcome.ready_by_4 ? 1U : 0U;
    for (std::size_t turn = 0; turn < regi.size(); ++turn) {
      regi[turn] += trial.by_turn[turn].regi ? 1U : 0U;
      vstar[turn] += trial.by_turn[turn].vstar ? 1U : 0U;
      active_vstar[turn] += trial.by_turn[turn].active_vstar ? 1U : 0U;
      energy[turn] += trial.by_turn[turn].energy ? 1U : 0U;
      payload[turn] += trial.by_turn[turn].payload ? 1U : 0U;
      k1[turn] += trial.by_turn[turn].k1 ? 1U : 0U;
    }
  }
};

struct CombinedScenarioAggregate {
  LetterAnalysisAggregate analysis;
  CheckpointAggregate baseline_checkpoints;
  CheckpointAggregate letter_checkpoints;
};

void add_letter_analysis(
    LetterAnalysisAggregate& aggregate,
    const TrialOutcome& baseline_outcome,
    const TrialOutcome& letter_outcome,
    const LetterFacts& facts) {
  ++aggregate.trials;
  aggregate.baseline_by2 += baseline_outcome.ready_by_2 ? 1U : 0U;
  aggregate.baseline_by3 += baseline_outcome.ready_by_3 ? 1U : 0U;
  aggregate.baseline_by4 += baseline_outcome.ready_by_4 ? 1U : 0U;
  aggregate.letter_by2 += letter_outcome.ready_by_2 ? 1U : 0U;
  aggregate.letter_by3 += letter_outcome.ready_by_3 ? 1U : 0U;
  aggregate.letter_by4 += letter_outcome.ready_by_4 ? 1U : 0U;
  aggregate.delta2.add(baseline_outcome.ready_by_2, letter_outcome.ready_by_2);
  aggregate.delta3.add(baseline_outcome.ready_by_3, letter_outcome.ready_by_3);
  aggregate.delta4.add(baseline_outcome.ready_by_4, letter_outcome.ready_by_4);

  aggregate.letter_total_plays += facts.total_plays;
  aggregate.locked_plays += facts.locked_plays;
  for (std::size_t index = 0; index < aggregate.letter_plays_by_turn.size(); ++index) {
    aggregate.letter_plays_by_turn[index] += facts.plays_by_turn[index];
  }
  for (const auto& [signature, count] : facts.signatures) {
    aggregate.signatures[signature] += count;
  }

  if (facts.played) {
    ++aggregate.letter_played_games;
    aggregate.played.add(baseline_outcome, letter_outcome);
    aggregate.branches[{facts.first_play_turn, facts.first_signature}]
        .add(baseline_outcome, letter_outcome);
  } else {
    aggregate.not_played.add(baseline_outcome, letter_outcome);
  }
}

CombinedScenarioAggregate analyze_scenario_combined(
    const Scenario& scenario, const DeckRecipe& baseline,
    const DeckRecipe& letter, const std::uint64_t trials,
    const std::uint64_t first_seed) {
  CombinedScenarioAggregate aggregate;
  for (std::uint64_t trial_index = 0; trial_index < trials; ++trial_index) {
    const std::uint64_t seed = first_seed + trial_index;
    const TrialWithCheckpoints baseline_trial =
        run_game_with_checkpoints(scenario, baseline, seed);

    TraceLog letter_trace{true, {}};
    const TrialWithCheckpoints letter_trial =
        run_game_with_checkpoints(scenario, letter, seed, &letter_trace);
    const LetterFacts facts = inspect_letter_trace(letter_trace, scenario);

    // Both evidence families consume this one matched-seed baseline/Letter pair.
    // Running a second 100k loop adds no modeled information and is the confirmed
    // CI overcomputation defect fixed here.
    // https://github.com/FlareZ123/pokemon-sims/issues/3008
    aggregate.baseline_checkpoints.add(baseline_trial);
    aggregate.letter_checkpoints.add(letter_trial);
    add_letter_analysis(
        aggregate.analysis, baseline_trial.outcome, letter_trial.outcome, facts);
  }
  return aggregate;
}

void write_checkpoint_header(std::ostream& out) {
  out << "variant,scenario,trials,ready_by_t2_pct,ready_by_t3_pct,"
         "ready_by_t4_pct,"
         "t2_regi_pct,t2_vstar_pct,t2_active_vstar_pct,t2_energy_pct,"
         "t2_payload_pct,t2_k1_pct,"
         "t3_regi_pct,t3_vstar_pct,t3_active_vstar_pct,t3_energy_pct,"
         "t3_payload_pct,t3_k1_pct,"
         "t4_regi_pct,t4_vstar_pct,t4_active_vstar_pct,t4_energy_pct,"
         "t4_payload_pct,t4_k1_pct\n";
}

void write_checkpoint_row(
    std::ostream& out, const std::string& variant,
    const Scenario& scenario, const CheckpointAggregate& aggregate) {
  out << variant << ',' << scenario.label << ',' << aggregate.trials << ','
      << std::fixed << std::setprecision(6)
      << pct(aggregate.by2, aggregate.trials) << ','
      << pct(aggregate.by3, aggregate.trials) << ','
      << pct(aggregate.by4, aggregate.trials);
  for (const std::size_t index : {1U, 2U, 3U}) {
    out << ',' << pct(aggregate.regi[index], aggregate.trials)
        << ',' << pct(aggregate.vstar[index], aggregate.trials)
        << ',' << pct(aggregate.active_vstar[index], aggregate.trials)
        << ',' << pct(aggregate.energy[index], aggregate.trials)
        << ',' << pct(aggregate.payload[index], aggregate.trials)
        << ',' << pct(aggregate.k1[index], aggregate.trials);
  }
  out << '\n';
}

void analyze_all_combined(
    const std::uint64_t trials, const std::uint64_t seed,
    const std::string& summary_path, const std::string& branch_path,
    const std::string& checkpoint_path) {
  const DeckRecipe baseline = baseline_recipe();
  const DeckRecipe letter = letter_recipe();
  verify_pair(baseline, letter);

  std::ofstream summary(summary_path, std::ios::binary | std::ios::trunc);
  std::ofstream branches(branch_path, std::ios::binary | std::ios::trunc);
  std::ofstream checkpoints(checkpoint_path, std::ios::binary | std::ios::trunc);
  if (!summary || !branches || !checkpoints) {
    throw std::runtime_error("could not open output");
  }
  write_summary_header(summary);
  write_branch_header(branches);
  write_checkpoint_header(checkpoints);

  const auto scenarios = all_scenarios();
  for (std::size_t index = 0; index < scenarios.size(); ++index) {
    const std::size_t seed_slot =
        index + (index >= 4 ? 1U : 0U) + (index >= 10 ? 1U : 0U);
    const CombinedScenarioAggregate aggregate = analyze_scenario_combined(
        scenarios[index], baseline, letter, trials,
        seed + 104729ULL * seed_slot);
    if (aggregate.analysis.locked_plays != 0U) {
      throw std::logic_error(
          "Professor's Letter was played while Item lock was active");
    }

    write_summary_row(summary, scenarios[index], aggregate.analysis);
    write_branch_rows(branches, scenarios[index], aggregate.analysis);
    write_checkpoint_row(
        checkpoints, "regidrago-shell", scenarios[index],
        aggregate.baseline_checkpoints);
    write_checkpoint_row(
        checkpoints, "letter-swap", scenarios[index],
        aggregate.letter_checkpoints);
  }
}

}  // namespace sim

int main(int argc, char** argv) {
  try {
    if (argc == 7 && std::string(argv[1]) == "--analyze-real-games") {
      const std::uint64_t trials = std::stoull(argv[2]);
      if (trials == 0) throw std::runtime_error("TRIALS must be positive");
      sim::analyze_all_combined(
          trials, std::stoull(argv[3]), argv[4], argv[5], argv[6]);
      return 0;
    }
    return professors_letter_real_game_analysis_legacy_main(argc, argv);
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

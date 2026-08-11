#define main ev_vs_qb_registered_scenarios_main
#include "ev_vs_quick_ball.cpp"
#undef main

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

int main() {
  const DeckRecipe vessel = vessel_recipe();
  const DeckRecipe quick_ball = quick_ball_recipe();
  std::ofstream aggregate("ev-vs-quick-ball-garbodor.csv", std::ios::binary | std::ios::trunc);
  std::ofstream paired("ev-vs-quick-ball-garbodor-paired.csv", std::ios::binary | std::ios::trunc);
  if (!aggregate || !paired) return 3;

  aggregate << "variant,scenario,attempted_trials,valid_trials,logic_error_pct,ready_by_t2_pct_valid,ready_by_t3_pct_valid,ready_by_t4_pct_valid,ready_by_t5_pct_valid,setup_failure_pct_valid\n";
  paired << "scenario,attempted_pairs,valid_pairs,invalid_pair_pct,qb_faster_pct_valid,vessel_faster_pct_valid,same_ready_turn_pct_valid,qb_minus_vessel_t2_pp_valid,qb_minus_vessel_t3_pp_valid,qb_minus_vessel_t4_pp_valid\n";

  std::size_t scenario_index = 0;
  for (const Scenario& scenario : sim::all_scenarios_with_garbodor()) {
    if (scenario.label.rfind("garbodor-shake-ability-lock/", 0) != 0) continue;
    const std::uint64_t scenario_seed = 22000000ULL + kScenarioStride * scenario_index++;
    Stats vessel_stats{};
    Stats qb_stats{};
    PairStats pair{};

    for (std::uint64_t trial = 0; trial < kTrials; ++trial) {
      const std::uint64_t seed = scenario_seed + trial;
      const RunResult vessel_run = run_one(scenario, vessel, seed);
      const RunResult qb_run = run_one(scenario, quick_ball, seed);
      accumulate(vessel_stats, vessel_run, seed);
      accumulate(qb_stats, qb_run, seed);
      ++pair.attempted;

      if (!vessel_run.outcome || !qb_run.outcome) continue;
      ++pair.valid;
      const int vessel_turn = comparable_ready_turn(*vessel_run.outcome);
      const int qb_turn = comparable_ready_turn(*qb_run.outcome);
      if (qb_turn < vessel_turn) {
        ++pair.qb_faster;
        if (pair.qb_witnesses.size() < kWitnessesPerDirection) {
          pair.qb_witnesses.push_back(seed);
        }
      } else if (vessel_turn < qb_turn) {
        ++pair.vessel_faster;
        if (pair.vessel_witnesses.size() < kWitnessesPerDirection) {
          pair.vessel_witnesses.push_back(seed);
        }
      } else {
        ++pair.same_ready_turn;
      }
    }

    const auto write_stats = [&](const std::string_view variant, const Stats& stats) {
      aggregate << variant << ',' << scenario.label << ',' << stats.attempted << ',' << stats.valid
                << ',' << std::fixed << std::setprecision(6) << pct(stats.logic_errors, stats.attempted)
                << ',' << pct(stats.by2, stats.valid) << ',' << pct(stats.by3, stats.valid)
                << ',' << pct(stats.by4, stats.valid) << ',' << pct(stats.by5, stats.valid)
                << ',' << pct(stats.failures, stats.valid) << '\n';
    };
    write_stats("1-ev-null", vessel_stats);
    write_stats("1-qb-null", qb_stats);

    paired << scenario.label << ',' << pair.attempted << ',' << pair.valid << ','
           << std::fixed << std::setprecision(6) << pct(pair.attempted - pair.valid, pair.attempted)
           << ',' << pct(pair.qb_faster, pair.valid) << ',' << pct(pair.vessel_faster, pair.valid)
           << ',' << pct(pair.same_ready_turn, pair.valid) << ','
           << (pct(qb_stats.by2, qb_stats.valid) - pct(vessel_stats.by2, vessel_stats.valid)) << ','
           << (pct(qb_stats.by3, qb_stats.valid) - pct(vessel_stats.by3, vessel_stats.valid)) << ','
           << (pct(qb_stats.by4, qb_stats.valid) - pct(vessel_stats.by4, vessel_stats.valid)) << '\n';

    const std::string scenario_name = sanitize(scenario.label);
    for (const std::uint64_t seed : pair.qb_witnesses) {
      write_trace("trace-" + scenario_name + "-qb-faster-" + std::to_string(seed) + "-vessel.txt",
                  scenario, vessel, seed);
      write_trace("trace-" + scenario_name + "-qb-faster-" + std::to_string(seed) + "-qb.txt",
                  scenario, quick_ball, seed);
    }
    for (const std::uint64_t seed : pair.vessel_witnesses) {
      write_trace("trace-" + scenario_name + "-vessel-faster-" + std::to_string(seed) + "-vessel.txt",
                  scenario, vessel, seed);
      write_trace("trace-" + scenario_name + "-vessel-faster-" + std::to_string(seed) + "-qb.txt",
                  scenario, quick_ball, seed);
    }
  }

  std::cout << "GARBODOR AGGREGATE\n";
  std::cout << std::ifstream("ev-vs-quick-ball-garbodor.csv").rdbuf();
  std::cout << "\nGARBODOR PAIRED\n";
  std::cout << std::ifstream("ev-vs-quick-ball-garbodor-paired.csv").rdbuf();
  return 0;
}

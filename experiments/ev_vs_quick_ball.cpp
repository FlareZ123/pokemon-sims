#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using sim::Card;
using sim::DeckRecipe;
using sim::Engine;
using sim::Scenario;
using sim::TraceLog;
using sim::TrialOutcome;

constexpr std::uint64_t kTrials = 100000;
constexpr std::uint64_t kBaseSeed = 20260810;
constexpr std::uint64_t kScenarioStride = 104729;
constexpr std::size_t kWitnessesPerDirection = 4;
constexpr std::size_t kExceptionWitnesses = 4;

struct Stats {
  std::uint64_t attempted{};
  std::uint64_t valid{};
  std::uint64_t logic_errors{};
  std::uint64_t by2{};
  std::uint64_t by3{};
  std::uint64_t by4{};
  std::uint64_t by5{};
  std::uint64_t failures{};
  std::uint64_t started_regi{};
  std::uint64_t started_tapu{};
  std::uint64_t mulligans{};
  std::vector<std::uint64_t> exception_witnesses;
};

struct PairStats {
  std::uint64_t attempted{};
  std::uint64_t valid{};
  std::uint64_t qb_faster{};
  std::uint64_t vessel_faster{};
  std::uint64_t same_ready_turn{};
  std::uint64_t both_fail_by4{};
  std::uint64_t vessel_only_error{};
  std::uint64_t qb_only_error{};
  std::uint64_t both_error{};
  std::vector<std::uint64_t> qb_witnesses;
  std::vector<std::uint64_t> vessel_witnesses;
};

struct RunResult {
  std::optional<TrialOutcome> outcome;
  std::string error;
};

void adjust(DeckRecipe& recipe, const Card card, const int delta) {
  const auto found = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  if (found == recipe.end()) {
    if (delta <= 0) throw std::logic_error("experiment cut missing");
    recipe.push_back({card, delta});
    return;
  }
  found->second += delta;
  if (found->second < 0) throw std::logic_error("experiment cut exceeds copies");
  if (found->second == 0) recipe.erase(found);
}

int copies(const DeckRecipe& recipe, const Card card) {
  const auto found = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  return found == recipe.end() ? 0 : found->second;
}

std::size_t card_count(const DeckRecipe& recipe) {
  return static_cast<std::size_t>(std::accumulate(
      recipe.begin(), recipe.end(), 0,
      [](const int total, const auto& entry) { return total + entry.second; }));
}

DeckRecipe vessel_recipe() {
  DeckRecipe recipe = sim::baseline_recipe();
  adjust(recipe, Card::EarthenVessel, -1);
  return recipe;
}

DeckRecipe quick_ball_recipe() {
  DeckRecipe recipe = sim::baseline_recipe();
  adjust(recipe, Card::EarthenVessel, -2);
  adjust(recipe, Card::QuickBall, +1);
  return recipe;
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(count) / static_cast<double>(total);
}

int comparable_ready_turn(const TrialOutcome& outcome) {
  return outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 4
      ? outcome.first_ready_turn
      : 99;
}

RunResult run_one(const Scenario& scenario, const DeckRecipe& recipe,
                  const std::uint64_t seed, TraceLog* trace = nullptr) {
  try {
    std::mt19937_64 rng(seed);
    Engine engine(scenario, recipe, rng, trace);
    return RunResult{engine.run(), {}};
  } catch (const std::logic_error& error) {
    return RunResult{std::nullopt, error.what()};
  }
}

void accumulate(Stats& stats, const RunResult& run, const std::uint64_t seed) {
  ++stats.attempted;
  if (!run.outcome) {
    ++stats.logic_errors;
    if (stats.exception_witnesses.size() < kExceptionWitnesses) {
      stats.exception_witnesses.push_back(seed);
    }
    return;
  }
  ++stats.valid;
  const TrialOutcome& outcome = *run.outcome;
  stats.by2 += outcome.ready_by_2 ? 1U : 0U;
  stats.by3 += outcome.ready_by_3 ? 1U : 0U;
  stats.by4 += outcome.ready_by_4 ? 1U : 0U;
  stats.by5 += outcome.ready_by_5 ? 1U : 0U;
  stats.failures += outcome.setup_failed ? 1U : 0U;
  stats.started_regi += outcome.started_regi ? 1U : 0U;
  stats.started_tapu += outcome.started_tapu ? 1U : 0U;
  stats.mulligans += outcome.mulligans;
}

std::string sanitize(std::string text) {
  for (char& c : text) {
    if (c == '/' || c == ' ' || c == ':') c = '_';
  }
  return text;
}

void write_trace(const std::string& path, const Scenario& scenario,
                 const DeckRecipe& recipe, const std::uint64_t seed) {
  TraceLog trace{true, {}};
  const RunResult run = run_one(scenario, recipe, seed, &trace);
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << "scenario: " << scenario.label << '\n';
  out << "seed: " << seed << '\n';
  if (!run.outcome) {
    out << "logic-error: " << run.error << "\n\n";
  } else {
    const TrialOutcome& outcome = *run.outcome;
    out << "first-ready turn: "
        << (outcome.first_ready_turn > 0 ? std::to_string(outcome.first_ready_turn) : "not ready")
        << '\n';
    out << "setup result: " << (outcome.setup_failed ? "failure" : "success") << "\n\n";
  }
  for (const std::string& line : trace.lines) out << line << '\n';
}

}  // namespace

int main() {
  const DeckRecipe vessel = vessel_recipe();
  const DeckRecipe quick_ball = quick_ball_recipe();

  if (card_count(vessel) != 59U || card_count(quick_ball) != 59U ||
      copies(vessel, Card::EarthenVessel) != 1 || copies(vessel, Card::QuickBall) != 3 ||
      copies(quick_ball, Card::EarthenVessel) != 0 || copies(quick_ball, Card::QuickBall) != 4) {
    std::cerr << "experiment recipe contract failed\n";
    return 2;
  }

  std::ofstream aggregate("ev-vs-quick-ball.csv", std::ios::binary | std::ios::trunc);
  std::ofstream paired("ev-vs-quick-ball-paired.csv", std::ios::binary | std::ios::trunc);
  std::ofstream witnesses("ev-vs-quick-ball-witnesses.csv", std::ios::binary | std::ios::trunc);
  if (!aggregate || !paired || !witnesses) return 3;

  aggregate << "variant,scenario,attempted_trials,valid_trials,logic_error_pct,effective_cards,quick_ball,earthen_vessel,ready_by_t2_pct_valid,ready_by_t3_pct_valid,ready_by_t4_pct_valid,ready_by_t5_pct_valid,setup_failure_pct_valid,started_regi_pct_valid,started_tapu_pct_valid,mean_mulligans_valid\n";
  paired << "scenario,attempted_pairs,valid_pairs,invalid_pair_pct,qb_faster_pct_valid,vessel_faster_pct_valid,same_ready_turn_pct_valid,both_fail_by4_pct_valid,qb_minus_vessel_t2_pp_valid,qb_minus_vessel_t3_pp_valid,qb_minus_vessel_t4_pp_valid,vessel_only_logic_error_pct,qb_only_logic_error_pct,both_logic_error_pct\n";
  witnesses << "scenario,direction,seed,vessel_ready_turn,quick_ball_ready_turn\n";

  const std::vector<Scenario> scenarios = sim::all_scenarios();
  for (std::size_t scenario_index = 0; scenario_index < scenarios.size(); ++scenario_index) {
    const Scenario& scenario = scenarios[scenario_index];
    const std::uint64_t scenario_seed = kBaseSeed + kScenarioStride * scenario_index;
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

      if (!vessel_run.outcome || !qb_run.outcome) {
        if (!vessel_run.outcome && !qb_run.outcome) {
          ++pair.both_error;
        } else if (!vessel_run.outcome) {
          ++pair.vessel_only_error;
        } else {
          ++pair.qb_only_error;
        }
        continue;
      }

      ++pair.valid;
      const int vessel_turn = comparable_ready_turn(*vessel_run.outcome);
      const int qb_turn = comparable_ready_turn(*qb_run.outcome);
      if (qb_turn < vessel_turn) {
        ++pair.qb_faster;
        if (pair.qb_witnesses.size() < kWitnessesPerDirection) {
          pair.qb_witnesses.push_back(seed);
          witnesses << scenario.label << ",quick-ball," << seed << ','
                    << (vessel_turn == 99 ? 0 : vessel_turn) << ',' << qb_turn << '\n';
        }
      } else if (vessel_turn < qb_turn) {
        ++pair.vessel_faster;
        if (pair.vessel_witnesses.size() < kWitnessesPerDirection) {
          pair.vessel_witnesses.push_back(seed);
          witnesses << scenario.label << ",earthen-vessel," << seed << ','
                    << vessel_turn << ',' << (qb_turn == 99 ? 0 : qb_turn) << '\n';
        }
      } else {
        ++pair.same_ready_turn;
      }
      if (vessel_turn == 99 && qb_turn == 99) ++pair.both_fail_by4;
    }

    const auto write_stats = [&](const std::string_view variant, const DeckRecipe& recipe,
                                 const Stats& stats) {
      aggregate << variant << ',' << scenario.label << ',' << stats.attempted << ','
                << stats.valid << ',' << std::fixed << std::setprecision(6)
                << pct(stats.logic_errors, stats.attempted) << ',' << card_count(recipe) << ','
                << copies(recipe, Card::QuickBall) << ',' << copies(recipe, Card::EarthenVessel)
                << ',' << pct(stats.by2, stats.valid) << ',' << pct(stats.by3, stats.valid) << ','
                << pct(stats.by4, stats.valid) << ',' << pct(stats.by5, stats.valid) << ','
                << pct(stats.failures, stats.valid) << ',' << pct(stats.started_regi, stats.valid)
                << ',' << pct(stats.started_tapu, stats.valid) << ','
                << (stats.valid == 0 ? 0.0 : static_cast<double>(stats.mulligans) /
                                            static_cast<double>(stats.valid)) << '\n';
    };
    write_stats("1-ev-null", vessel, vessel_stats);
    write_stats("1-qb-null", quick_ball, qb_stats);

    const std::uint64_t invalid_pairs = pair.attempted - pair.valid;
    paired << scenario.label << ',' << pair.attempted << ',' << pair.valid << ','
           << std::fixed << std::setprecision(6) << pct(invalid_pairs, pair.attempted) << ','
           << pct(pair.qb_faster, pair.valid) << ',' << pct(pair.vessel_faster, pair.valid) << ','
           << pct(pair.same_ready_turn, pair.valid) << ',' << pct(pair.both_fail_by4, pair.valid) << ','
           << (pct(qb_stats.by2, qb_stats.valid) - pct(vessel_stats.by2, vessel_stats.valid)) << ','
           << (pct(qb_stats.by3, qb_stats.valid) - pct(vessel_stats.by3, vessel_stats.valid)) << ','
           << (pct(qb_stats.by4, qb_stats.valid) - pct(vessel_stats.by4, vessel_stats.valid)) << ','
           << pct(pair.vessel_only_error, pair.attempted) << ','
           << pct(pair.qb_only_error, pair.attempted) << ','
           << pct(pair.both_error, pair.attempted) << '\n';

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
    for (const std::uint64_t seed : vessel_stats.exception_witnesses) {
      write_trace("trace-" + scenario_name + "-vessel-logic-error-" + std::to_string(seed) + ".txt",
                  scenario, vessel, seed);
    }
    for (const std::uint64_t seed : qb_stats.exception_witnesses) {
      write_trace("trace-" + scenario_name + "-qb-logic-error-" + std::to_string(seed) + ".txt",
                  scenario, quick_ball, seed);
    }
  }

  std::cout << "Wrote paired 1-EV-null vs 1-QB-null experiment over "
            << sim::all_scenarios().size() << " scenarios x " << kTrials
            << " attempted pairs, with simulator logic errors reported separately.\n";
  return 0;
}

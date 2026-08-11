#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
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

struct Stats {
  std::uint64_t trials{};
  std::uint64_t by2{};
  std::uint64_t by3{};
  std::uint64_t by4{};
  std::uint64_t by5{};
  std::uint64_t failures{};
  std::uint64_t started_regi{};
  std::uint64_t started_tapu{};
  std::uint64_t mulligans{};
};

struct PairStats {
  std::uint64_t qb_faster{};
  std::uint64_t vessel_faster{};
  std::uint64_t same_ready_turn{};
  std::uint64_t both_fail_by4{};
  std::vector<std::uint64_t> qb_witnesses;
  std::vector<std::uint64_t> vessel_witnesses;
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

void accumulate(Stats& stats, const TrialOutcome& outcome) {
  ++stats.trials;
  stats.by2 += outcome.ready_by_2 ? 1U : 0U;
  stats.by3 += outcome.ready_by_3 ? 1U : 0U;
  stats.by4 += outcome.ready_by_4 ? 1U : 0U;
  stats.by5 += outcome.ready_by_5 ? 1U : 0U;
  stats.failures += outcome.setup_failed ? 1U : 0U;
  stats.started_regi += outcome.started_regi ? 1U : 0U;
  stats.started_tapu += outcome.started_tapu ? 1U : 0U;
  stats.mulligans += outcome.mulligans;
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(count) / static_cast<double>(total);
}

int comparable_ready_turn(const TrialOutcome& outcome) {
  return outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 4
      ? outcome.first_ready_turn
      : 99;
}

TrialOutcome run_one(const Scenario& scenario, const DeckRecipe& recipe,
                     const std::uint64_t seed, TraceLog* trace = nullptr) {
  std::mt19937_64 rng(seed);
  Engine engine(scenario, recipe, rng, trace);
  return engine.run();
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
  const TrialOutcome outcome = run_one(scenario, recipe, seed, &trace);
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << "scenario: " << scenario.label << '\n';
  out << "seed: " << seed << '\n';
  out << "first-ready turn: "
      << (outcome.first_ready_turn > 0 ? std::to_string(outcome.first_ready_turn) : "not ready")
      << '\n';
  out << "setup result: " << (outcome.setup_failed ? "failure" : "success") << "\n\n";
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

  aggregate << "variant,scenario,trials,effective_cards,quick_ball,earthen_vessel,ready_by_t2_pct,ready_by_t3_pct,ready_by_t4_pct,ready_by_t5_pct,setup_failure_pct,started_regi_pct,started_tapu_pct,mean_mulligans\n";
  paired << "scenario,trials,qb_faster_pct,vessel_faster_pct,same_ready_turn_pct,both_fail_by4_pct,qb_minus_vessel_t2_pp,qb_minus_vessel_t3_pp,qb_minus_vessel_t4_pp\n";
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
      const TrialOutcome vessel_outcome = run_one(scenario, vessel, seed);
      const TrialOutcome qb_outcome = run_one(scenario, quick_ball, seed);
      accumulate(vessel_stats, vessel_outcome);
      accumulate(qb_stats, qb_outcome);

      const int vessel_turn = comparable_ready_turn(vessel_outcome);
      const int qb_turn = comparable_ready_turn(qb_outcome);
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
      aggregate << variant << ',' << scenario.label << ',' << stats.trials << ','
                << card_count(recipe) << ',' << copies(recipe, Card::QuickBall) << ','
                << copies(recipe, Card::EarthenVessel) << ',' << std::fixed << std::setprecision(6)
                << pct(stats.by2, stats.trials) << ',' << pct(stats.by3, stats.trials) << ','
                << pct(stats.by4, stats.trials) << ',' << pct(stats.by5, stats.trials) << ','
                << pct(stats.failures, stats.trials) << ',' << pct(stats.started_regi, stats.trials)
                << ',' << pct(stats.started_tapu, stats.trials) << ','
                << static_cast<double>(stats.mulligans) / static_cast<double>(stats.trials) << '\n';
    };
    write_stats("1-ev-null", vessel, vessel_stats);
    write_stats("1-qb-null", quick_ball, qb_stats);

    paired << scenario.label << ',' << kTrials << ',' << std::fixed << std::setprecision(6)
           << pct(pair.qb_faster, kTrials) << ',' << pct(pair.vessel_faster, kTrials) << ','
           << pct(pair.same_ready_turn, kTrials) << ',' << pct(pair.both_fail_by4, kTrials) << ','
           << (pct(qb_stats.by2, kTrials) - pct(vessel_stats.by2, kTrials)) << ','
           << (pct(qb_stats.by3, kTrials) - pct(vessel_stats.by3, kTrials)) << ','
           << (pct(qb_stats.by4, kTrials) - pct(vessel_stats.by4, kTrials)) << '\n';

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

  std::cout << "Wrote paired 1-EV-null vs 1-QB-null experiment over "
            << sim::all_scenarios().size() << " scenarios x " << kTrials << " trials.\n";
  return 0;
}

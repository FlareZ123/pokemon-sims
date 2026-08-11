#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
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
using sim::TrialOutcome;

constexpr std::uint64_t kTrials = 100000;
constexpr std::uint64_t kBaseSeed = 20260811;
constexpr std::uint64_t kScenarioStride = 104729;

struct Stats {
  std::uint64_t attempted{};
  std::uint64_t valid{};
  std::uint64_t logic_errors{};
  std::array<std::uint64_t, 4> ready{};
  std::uint64_t failures{};
  std::uint64_t started_regi{};
  std::uint64_t started_tapu{};
  std::uint64_t mulligans{};
};

struct PairStats {
  std::uint64_t attempted{};
  std::uint64_t valid{};
  std::uint64_t qb_faster{};
  std::uint64_t vessel_faster{};
  std::uint64_t same_ready_turn{};
  std::array<std::uint64_t, 4> qb_only_ready{};
  std::array<std::uint64_t, 4> vessel_only_ready{};
};

struct RunResult {
  std::optional<TrialOutcome> outcome;
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
  // The control replaces one Earthen Vessel with Mawile-GX as the experiment's
  // inert Basic proxy. Mawile-GX's only setup-adjacent Ability acts on the opposing
  // hand, which this setup model does not simulate, so it supplies Basic-card/opening
  // structure without supplying a Regidrago setup axis.
  // Mawile-GX: https://api.pokemontcg.io/v2/cards/sm11-141
  // Opponent-action model boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  adjust(recipe, Card::EarthenVessel, -1);
  adjust(recipe, Card::MawileGX, +1);
  return recipe;
}

DeckRecipe quick_ball_recipe() {
  DeckRecipe recipe = vessel_recipe();
  // Treatment changes only the remaining Earthen Vessel into Quick Ball. Quick
  // Ball can find Oricorio, whose Vital Dance searches up to two Basic Energy,
  // while retaining access to other Basic Pokemon such as Regidrago V and Tapu
  // Lele-GX. This is the exact compression/flexibility hypothesis under test.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  adjust(recipe, Card::EarthenVessel, -1);
  adjust(recipe, Card::QuickBall, +1);
  return recipe;
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 : 100.0 * static_cast<double>(count) / static_cast<double>(total);
}

int comparable_ready_turn(const TrialOutcome& outcome) {
  return outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 5
      ? outcome.first_ready_turn
      : 99;
}

RunResult run_one(const Scenario& scenario, const DeckRecipe& recipe,
                  const std::uint64_t seed) {
  try {
    std::mt19937_64 rng(seed);
    Engine engine(scenario, recipe, rng, nullptr);
    return RunResult{engine.run()};
  } catch (const std::logic_error&) {
    return RunResult{std::nullopt};
  }
}

std::array<bool, 4> readiness(const TrialOutcome& outcome) {
  return {outcome.ready_by_2, outcome.ready_by_3, outcome.ready_by_4, outcome.ready_by_5};
}

void accumulate(Stats& stats, const RunResult& run) {
  ++stats.attempted;
  if (!run.outcome) {
    ++stats.logic_errors;
    return;
  }
  ++stats.valid;
  const auto ready = readiness(*run.outcome);
  for (std::size_t i = 0; i < ready.size(); ++i) stats.ready[i] += ready[i] ? 1U : 0U;
  stats.failures += run.outcome->setup_failed ? 1U : 0U;
  stats.started_regi += run.outcome->started_regi ? 1U : 0U;
  stats.started_tapu += run.outcome->started_tapu ? 1U : 0U;
  stats.mulligans += run.outcome->mulligans;
}

void accumulate_pair(PairStats& pair, const RunResult& vessel_run, const RunResult& qb_run) {
  ++pair.attempted;
  if (!vessel_run.outcome || !qb_run.outcome) return;
  ++pair.valid;

  const int vessel_turn = comparable_ready_turn(*vessel_run.outcome);
  const int qb_turn = comparable_ready_turn(*qb_run.outcome);
  if (qb_turn < vessel_turn) {
    ++pair.qb_faster;
  } else if (vessel_turn < qb_turn) {
    ++pair.vessel_faster;
  } else {
    ++pair.same_ready_turn;
  }

  const auto vessel_ready = readiness(*vessel_run.outcome);
  const auto qb_ready = readiness(*qb_run.outcome);
  for (std::size_t i = 0; i < vessel_ready.size(); ++i) {
    if (qb_ready[i] && !vessel_ready[i]) ++pair.qb_only_ready[i];
    if (vessel_ready[i] && !qb_ready[i]) ++pair.vessel_only_ready[i];
  }
}

}  // namespace

int main() {
  const DeckRecipe vessel = vessel_recipe();
  const DeckRecipe quick_ball = quick_ball_recipe();

  if (card_count(vessel) != 60U || card_count(quick_ball) != 60U ||
      copies(vessel, Card::MawileGX) != 1 || copies(quick_ball, Card::MawileGX) != 1 ||
      copies(vessel, Card::EarthenVessel) != 1 || copies(vessel, Card::QuickBall) != 3 ||
      copies(quick_ball, Card::EarthenVessel) != 0 || copies(quick_ball, Card::QuickBall) != 4) {
    std::cerr << "experiment recipe contract failed\n";
    return 2;
  }

  std::ofstream aggregate("ev-qb-null-basic.csv", std::ios::binary | std::ios::trunc);
  std::ofstream paired("ev-qb-null-basic-paired.csv", std::ios::binary | std::ios::trunc);
  if (!aggregate || !paired) return 3;

  aggregate << "variant,scenario,attempted,valid,logic_error_pct,cards,null_basic,quick_ball,earthen_vessel,ready_t2_pct,ready_t3_pct,ready_t4_pct,ready_t5_pct,failure_pct,started_regi_pct,started_tapu_pct,mean_mulligans\n";
  paired << "scenario,attempted_pairs,valid_pairs,invalid_pair_pct,qb_faster_pct,vessel_faster_pct,same_ready_turn_pct,qb_minus_vessel_t2_pp,qb_minus_vessel_t3_pp,qb_minus_vessel_t4_pp,qb_minus_vessel_t5_pp,qb_only_t2,vessel_only_t2,qb_only_t3,vessel_only_t3,qb_only_t4,vessel_only_t4,qb_only_t5,vessel_only_t5\n";

  const std::vector<Scenario> scenarios = sim::all_scenarios_with_garbodor();
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
      accumulate(vessel_stats, vessel_run);
      accumulate(qb_stats, qb_run);
      accumulate_pair(pair, vessel_run, qb_run);
    }

    const auto write_stats = [&](const std::string_view variant, const DeckRecipe& recipe,
                                 const Stats& stats) {
      aggregate << variant << ',' << scenario.label << ',' << stats.attempted << ',' << stats.valid
                << ',' << std::fixed << std::setprecision(6) << pct(stats.logic_errors, stats.attempted)
                << ',' << card_count(recipe) << ',' << copies(recipe, Card::MawileGX)
                << ',' << copies(recipe, Card::QuickBall) << ',' << copies(recipe, Card::EarthenVessel)
                << ',' << pct(stats.ready[0], stats.valid) << ',' << pct(stats.ready[1], stats.valid)
                << ',' << pct(stats.ready[2], stats.valid) << ',' << pct(stats.ready[3], stats.valid)
                << ',' << pct(stats.failures, stats.valid) << ',' << pct(stats.started_regi, stats.valid)
                << ',' << pct(stats.started_tapu, stats.valid) << ','
                << (stats.valid == 0 ? 0.0 : static_cast<double>(stats.mulligans) /
                                            static_cast<double>(stats.valid)) << '\n';
    };

    write_stats("1ev+null-basic", vessel, vessel_stats);
    write_stats("1qb+null-basic", quick_ball, qb_stats);

    paired << scenario.label << ',' << pair.attempted << ',' << pair.valid << ','
           << std::fixed << std::setprecision(6) << pct(pair.attempted - pair.valid, pair.attempted)
           << ',' << pct(pair.qb_faster, pair.valid) << ',' << pct(pair.vessel_faster, pair.valid)
           << ',' << pct(pair.same_ready_turn, pair.valid);
    for (std::size_t i = 0; i < 4; ++i) {
      paired << ',' << (pct(qb_stats.ready[i], qb_stats.valid) -
                         pct(vessel_stats.ready[i], vessel_stats.valid));
    }
    for (std::size_t i = 0; i < 4; ++i) {
      paired << ',' << pair.qb_only_ready[i] << ',' << pair.vessel_only_ready[i];
    }
    paired << '\n';
  }

  std::cout << "NULL-BASIC EARTHEN VESSEL VS QUICK BALL AGGREGATE\n";
  std::cout << std::ifstream("ev-qb-null-basic.csv").rdbuf();
  std::cout << "\nNULL-BASIC EARTHEN VESSEL VS QUICK BALL PAIRED\n";
  std::cout << std::ifstream("ev-qb-null-basic-paired.csv").rdbuf();
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr std::uint64_t kScreenTrials = 25000;
constexpr std::uint64_t kFinalTrials = 75000;
constexpr std::uint64_t kSeed = 20260810;
constexpr std::size_t kFinalistCount = 6;

struct Candidate {
  const char* label;
  sim::Card cut;
};

// Candidate counts come from the current Regidrago-shell recipe:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// This screen intentionally excludes Dragon payloads, Regidrago V/VSTAR counts,
// matchup/lock cards, stadiums, and recovery/disruption cards. Only setup,
// acceleration, search, draw, mobility, and connector pieces are eligible.
constexpr std::array<Candidate, 19> kCandidates{{
    {"Oricorio GRI 55", sim::Card::Oricorio},
    {"Latias ex", sim::Card::LatiasEx},
    {"Tapu Lele-GX", sim::Card::TapuLeleGX},
    {"Brilliant Blender", sim::Card::BrilliantBlender},
    {"Mysterious Treasure", sim::Card::MysteriousTreasure},
    {"Quick Ball", sim::Card::QuickBall},
    {"Earthen Vessel", sim::Card::EarthenVessel},
    {"Arven", sim::Card::Arven},
    {"Crispin", sim::Card::Crispin},
    {"Professor Burnet", sim::Card::ProfessorBurnet},
    {"Serena", sim::Card::Serena},
    {"Tate & Liza", sim::Card::TateLiza},
    {"Steven's Resolve", sim::Card::StevensResolve},
    {"Gladion", sim::Card::Gladion},
    {"Forest Seal Stone", sim::Card::ForestSealStone},
    {"Powerglass", sim::Card::Powerglass},
    {"Hisuian Heavy Ball", sim::Card::HisuianHeavyBall},
    {"Grass Energy", sim::Card::Grass},
    {"Fire Energy", sim::Card::Fire},
}};

struct Deltas {
  double t2{};
  double t3{};
  double t4{};
};

struct RankedCandidate {
  Candidate candidate{};
  double worst_loss{};
  double mean_loss{};
};

sim::DeckRecipe recipe_with_minior(const Candidate& candidate) {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::adjust_modeling_recipe(recipe, candidate.cut, -1);

  // Minior PAR 99 is a Basic Pokemon with Retreat Cost 1. Its Ability and attack
  // are deliberately ignored because this experiment measures setup-slot cost:
  // https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sv-series/sv04/99/
  // Mawile-GX is used only as the simulator's inert Basic/Retreat-1/non-Mysterious-
  // Treasure proxy in the unlocked shell scenarios screened here:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
  sim::adjust_modeling_recipe(recipe, sim::Card::MawileGX, 1);

  std::string error;
  if (!sim::validate_recipe({std::string("safe-cut-") + candidate.label, recipe}, &error)) {
    throw std::logic_error(error);
  }
  return recipe;
}

double rate(const std::uint64_t count, const std::uint64_t trials) {
  return 100.0 * static_cast<double>(count) / static_cast<double>(trials);
}

Deltas deltas(const sim::Aggregate& baseline,
              const sim::Aggregate& candidate,
              const std::uint64_t trials) {
  return {
      rate(candidate.by2, trials) - rate(baseline.by2, trials),
      rate(candidate.by3, trials) - rate(baseline.by3, trials),
      rate(candidate.by4, trials) - rate(baseline.by4, trials),
  };
}

double loss(const double delta) {
  return delta < 0.0 ? -delta : 0.0;
}

void print_result(const char* phase,
                  const Candidate& candidate,
                  const sim::Scenario& scenario,
                  const std::uint64_t trials,
                  const sim::Aggregate& baseline,
                  const sim::Aggregate& result) {
  const Deltas d = deltas(baseline, result, trials);
  std::cout << phase << ",result,\"" << candidate.label << "\"," << scenario.label << ',' << trials << ','
            << std::fixed << std::setprecision(6)
            << rate(baseline.by2, trials) << ',' << rate(result.by2, trials) << ',' << d.t2 << ','
            << rate(baseline.by3, trials) << ',' << rate(result.by3, trials) << ',' << d.t3 << ','
            << rate(baseline.by4, trials) << ',' << rate(result.by4, trials) << ',' << d.t4 << "\n";
}

}  // namespace

int main() {
  constexpr std::array<const char*, 2> screen_labels{
      "strict-jit/go-first",
      "strict-jit/go-second",
  };
  constexpr std::array<const char*, 6> final_labels{
      "strict-jit/go-first",
      "matchup-flex-jit/go-first",
      "no-discard-control/go-first",
      "strict-jit/go-second",
      "matchup-flex-jit/go-second",
      "no-discard-control/go-second",
  };

  std::cout << "phase,row_type,candidate,scenario,trials,baseline_t2_pct,candidate_t2_pct,delta_t2_pp,"
               "baseline_t3_pct,candidate_t3_pct,delta_t3_pp,baseline_t4_pct,candidate_t4_pct,delta_t4_pp\n";

  const sim::DeckRecipe baseline_recipe = sim::baseline_recipe();
  std::array<sim::Aggregate, screen_labels.size()> screen_baselines{};
  std::array<sim::Scenario, screen_labels.size()> screen_scenarios{};

  for (std::size_t i = 0; i < screen_labels.size(); ++i) {
    const auto scenario = sim::scenario_by_label(std::string(screen_labels[i]));
    if (!scenario) throw std::logic_error("missing strict-JIT screen scenario");
    screen_scenarios[i] = *scenario;
    screen_baselines[i] = sim::simulate(*scenario, baseline_recipe, kScreenTrials, kSeed + 104729ULL * i);
  }

  std::vector<RankedCandidate> ranking;
  ranking.reserve(kCandidates.size());
  for (const Candidate& candidate : kCandidates) {
    const sim::DeckRecipe recipe = recipe_with_minior(candidate);
    double worst_loss = 0.0;
    double total_loss = 0.0;
    std::size_t metric_count = 0;
    for (std::size_t i = 0; i < screen_labels.size(); ++i) {
      const std::uint64_t common_seed = kSeed + 104729ULL * i;
      const sim::Aggregate result = sim::simulate(screen_scenarios[i], recipe, kScreenTrials, common_seed);
      const Deltas d = deltas(screen_baselines[i], result, kScreenTrials);
      print_result("screen", candidate, screen_scenarios[i], kScreenTrials, screen_baselines[i], result);
      for (const double value : {d.t2, d.t3, d.t4}) {
        const double metric_loss = loss(value);
        worst_loss = std::max(worst_loss, metric_loss);
        total_loss += metric_loss;
        ++metric_count;
      }
    }
    ranking.push_back({candidate, worst_loss, total_loss / static_cast<double>(metric_count)});
  }

  std::sort(ranking.begin(), ranking.end(), [](const RankedCandidate& lhs, const RankedCandidate& rhs) {
    if (lhs.worst_loss != rhs.worst_loss) return lhs.worst_loss < rhs.worst_loss;
    if (lhs.mean_loss != rhs.mean_loss) return lhs.mean_loss < rhs.mean_loss;
    return std::string(lhs.candidate.label) < std::string(rhs.candidate.label);
  });

  for (std::size_t i = 0; i < ranking.size(); ++i) {
    std::cout << "screen,ranking,\"" << ranking[i].candidate.label << "\",rank-" << (i + 1)
              << ",0,0,0," << -ranking[i].worst_loss << ",0,0," << -ranking[i].mean_loss
              << ",0,0,0\n";
  }

  std::array<sim::Aggregate, final_labels.size()> final_baselines{};
  std::array<sim::Scenario, final_labels.size()> final_scenarios{};
  for (std::size_t i = 0; i < final_labels.size(); ++i) {
    const auto scenario = sim::scenario_by_label(std::string(final_labels[i]));
    if (!scenario) throw std::logic_error("missing unlocked finalist scenario");
    final_scenarios[i] = *scenario;
    final_baselines[i] = sim::simulate(*scenario, baseline_recipe, kFinalTrials, kSeed + 1000003ULL + 104729ULL * i);
  }

  for (std::size_t finalist = 0; finalist < std::min(kFinalistCount, ranking.size()); ++finalist) {
    const Candidate candidate = ranking[finalist].candidate;
    const sim::DeckRecipe recipe = recipe_with_minior(candidate);
    for (std::size_t i = 0; i < final_labels.size(); ++i) {
      const std::uint64_t common_seed = kSeed + 1000003ULL + 104729ULL * i;
      const sim::Aggregate result = sim::simulate(final_scenarios[i], recipe, kFinalTrials, common_seed);
      print_result("final", candidate, final_scenarios[i], kFinalTrials, final_baselines[i], result);
    }
  }

  return 0;
}

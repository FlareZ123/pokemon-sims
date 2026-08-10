#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace {

constexpr std::uint64_t kTrials = 100000;
constexpr std::uint64_t kSeed = 20260810;

sim::DeckRecipe minior_proxy_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();

  // Baseline slot being tested: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
  sim::adjust_modeling_recipe(recipe, sim::Card::Oricorio, -1);

  // Minior PAR 99 is a Basic Pokemon with Retreat Cost 1. Its Ability and attack are intentionally ignored here:
  // https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sv-series/sv04/99/
  // Mawile-GX is used only as the simulator's inert Basic/Retreat-1/non-Mysterious-Treasure proxy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
  sim::adjust_modeling_recipe(recipe, sim::Card::MawileGX, 1);

  std::string error;
  if (!sim::validate_recipe({"regidrago-shell-minior-proxy", recipe}, &error)) {
    throw std::logic_error(error);
  }
  return recipe;
}

void print_row(const sim::Scenario& scenario,
               const sim::Aggregate& baseline,
               const sim::Aggregate& minior) {
  const auto rate = [](const std::uint64_t count) {
    return 100.0 * static_cast<double>(count) / static_cast<double>(kTrials);
  };

  const double baseline_t2 = rate(baseline.by2);
  const double minior_t2 = rate(minior.by2);
  const double baseline_t3 = rate(baseline.by3);
  const double minior_t3 = rate(minior.by3);
  const double baseline_t4 = rate(baseline.by4);
  const double minior_t4 = rate(minior.by4);

  std::cout << scenario.label << ',' << kTrials << ','
            << std::fixed << std::setprecision(6)
            << baseline_t2 << ',' << minior_t2 << ',' << (minior_t2 - baseline_t2) << ','
            << baseline_t3 << ',' << minior_t3 << ',' << (minior_t3 - baseline_t3) << ','
            << baseline_t4 << ',' << minior_t4 << ',' << (minior_t4 - baseline_t4) << '\n';
}

}  // namespace

int main() {
  const sim::DeckRecipe baseline = sim::baseline_recipe();
  const sim::DeckRecipe minior = minior_proxy_recipe();
  constexpr std::array<std::string_view, 6> scenario_labels{
      "strict-jit/go-first",
      "matchup-flex-jit/go-first",
      "no-discard-control/go-first",
      "strict-jit/go-second",
      "matchup-flex-jit/go-second",
      "no-discard-control/go-second",
  };

  std::cout << "scenario,trials,baseline_t2_pct,minior_t2_pct,delta_t2_pp,"
               "baseline_t3_pct,minior_t3_pct,delta_t3_pp,"
               "baseline_t4_pct,minior_t4_pct,delta_t4_pp\n";

  for (std::size_t scenario_index = 0; scenario_index < scenario_labels.size(); ++scenario_index) {
    const auto scenario = sim::scenario_by_label(scenario_labels[scenario_index]);
    if (!scenario) throw std::logic_error("missing unlocked shell scenario");

    // Baseline and Minior use the same per-scenario random stream for a paired comparison:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
    const std::uint64_t common_seed = kSeed + 104729ULL * scenario_index;
    const sim::Aggregate baseline_result = sim::simulate(*scenario, baseline, kTrials, common_seed);
    const sim::Aggregate minior_result = sim::simulate(*scenario, minior, kTrials, common_seed);
    print_row(*scenario, baseline_result, minior_result);
  }

  return 0;
}

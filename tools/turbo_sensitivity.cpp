#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>

namespace {

sim::DeckRecipe replace_copies(const sim::DeckRecipe& base, const sim::Card removed,
                               const int count) {
  sim::DeckRecipe recipe = base;
  int remaining = count;
  for (auto& [card, copies] : recipe) {
    if (card != removed) continue;
    const int take = std::min(copies, remaining);
    copies -= take;
    remaining -= take;
    if (remaining == 0) break;
  }
  if (remaining != 0) throw std::runtime_error("replacement count exceeds deck copies");
  recipe.erase(std::remove_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.second == 0;
  }), recipe.end());
  recipe.emplace_back(sim::Card::TurboBlank, count);
  return recipe;
}

void emit(const std::string& mode, const sim::Card card, const int replaced,
          const sim::Scenario& scenario, const sim::Aggregate& baseline,
          const sim::Aggregate& variant) {
  std::cout << mode << ',' << '"' << sim::name(card) << '"' << ',' << replaced << ','
            << '"' << scenario.label << '"' << ',' << variant.trials << ','
            << sim::pct(baseline.by2, baseline.trials) << ','
            << sim::pct(variant.by2, variant.trials) << ','
            << sim::pct(variant.by2, variant.trials) - sim::pct(baseline.by2, baseline.trials) << ','
            << sim::pct(baseline.by3, baseline.trials) << ','
            << sim::pct(variant.by3, variant.trials) << ','
            << sim::pct(variant.by3, variant.trials) - sim::pct(baseline.by3, baseline.trials) << ','
            << sim::pct(baseline.by4, baseline.trials) << ','
            << sim::pct(variant.by4, variant.trials) << ','
            << sim::pct(variant.by4, variant.trials) - sim::pct(baseline.by4, baseline.trials) << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  const std::uint64_t trials = argc > 1 ? std::stoull(argv[1]) : 500;
  const std::uint64_t seed = argc > 2 ? std::stoull(argv[2]) : 20260716;
  const std::size_t start = argc > 3 ? std::stoull(argv[3]) : 0;
  const sim::DeckRecipe base = sim::baseline_recipe();
  const std::size_t requested_end = argc > 4 ? std::stoull(argv[4]) : base.size();
  const std::size_t end = std::min(requested_end, base.size());
  if (start >= end) throw std::runtime_error("empty sensitivity shard");

  const auto scenarios = sim::all_scenarios();
  std::vector<sim::Aggregate> baselines;
  baselines.reserve(scenarios.size());
  for (std::size_t i = 0; i < scenarios.size(); ++i) {
    baselines.push_back(sim::simulate(scenarios[i], base, trials, seed + 104729ULL * i));
  }

  // Earliest-ready objective and scenario matrix:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
  // The inert filler keeps every sensitivity variant at exactly 60 cards.
  std::cout << "mode,card,replaced_copies,scenario,trials,baseline_t2_pct,variant_t2_pct,delta_t2_pp,baseline_t3_pct,variant_t3_pct,delta_t3_pp,baseline_t4_pct,variant_t4_pct,delta_t4_pp\n";
  for (std::size_t card_index = start; card_index < end; ++card_index) {
    const auto [card, copies] = base[card_index];
    const std::array<std::pair<std::string, int>, 2> modes{{
        {"one_copy", 1}, {"all_copies", copies},
    }};
    for (const auto& [mode, replace_count] : modes) {
      const sim::DeckRecipe variant = replace_copies(base, card, replace_count);
      for (std::size_t i = 0; i < scenarios.size(); ++i) {
        const sim::Aggregate result = sim::simulate(
            scenarios[i], variant, trials, seed + 104729ULL * i);
        emit(mode, card, replace_count, scenarios[i], baselines[i], result);
      }
    }
  }
  return 0;
}

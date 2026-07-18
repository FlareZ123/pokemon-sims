#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

sim::DeckRecipe pokemon_communication_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto mawile = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == sim::Card::MawileGX;
  });
  if (mawile == recipe.end() || mawile->second != 1) {
    throw std::runtime_error("The one-card Pokémon Communication variant requires the baseline Mawile-GX singleton.");
  }

  // Keep the submitted baseline unchanged and evaluate the same explicit variant used
  // by the canonical regression: Mawile-GX -1, Pokémon Communication +1.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Variant contract: https://github.com/FlareZ123/pokemon-sims/issues/701
  // Canonical test: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/pokemon_communication_tests.cpp#L118-L176
  recipe.erase(mawile);
  recipe.emplace_back(sim::Card::PokemonCommunication, 1);
  return recipe;
}

void print_row(const std::string_view scenario_label, const std::string_view deck_label,
               const sim::Aggregate& aggregate) {
  std::cout << scenario_label << ',' << deck_label << ',' << aggregate.trials << ','
            << sim::pct(aggregate.by2, aggregate.trials) << ','
            << sim::standard_error_pp(aggregate.by2, aggregate.trials) << ','
            << sim::pct(aggregate.by3, aggregate.trials) << ','
            << sim::standard_error_pp(aggregate.by3, aggregate.trials) << ','
            << sim::pct(aggregate.by4, aggregate.trials) << ','
            << sim::standard_error_pp(aggregate.by4, aggregate.trials) << '\n';
}

}  // namespace

int main() {
  constexpr std::uint64_t trials = 100000;
  constexpr std::uint64_t base_seed = 20260717;
  constexpr std::uint64_t seed_stride = 104729;
  constexpr std::array<std::string_view, 14> scenario_labels{{
      "strict-jit/go-first",
      "matchup-flex-jit/go-first",
      "no-discard-control/go-first",
      "strict-jit-turn2-item-lock/go-first",
      "strict-jit-full-item-lock/go-first",
      "strict-jit-rulebox-ability-lock/go-first",
      "strict-jit-combined-lock/go-first",
      "strict-jit/go-second",
      "matchup-flex-jit/go-second",
      "no-discard-control/go-second",
      "strict-jit-turn2-item-lock/go-second",
      "strict-jit-full-item-lock/go-second",
      "strict-jit-rulebox-ability-lock/go-second",
      "strict-jit-combined-lock/go-second",
  }};

  const sim::DeckRecipe baseline = sim::baseline_recipe();
  const sim::DeckRecipe communication = pokemon_communication_recipe();

  // Use identical scenario seeds for both lists. The comparison covers every
  // repository-defined lock and turn-order condition under the earliest-ready policy.
  // Probability contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#what-the-probability-means
  // Scenario policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  std::cout << std::fixed << std::setprecision(6);
  std::cout << "scenario,deck,trials,ready_by_t2_pct,ready_by_t2_se_pp,ready_by_t3_pct,ready_by_t3_se_pp,ready_by_t4_pct,ready_by_t4_se_pp\n";
  for (std::size_t index = 0; index < scenario_labels.size(); ++index) {
    const auto scenario = sim::scenario_by_label(std::string(scenario_labels[index]));
    if (!scenario) throw std::runtime_error("Unknown scenario in Pokémon Communication audit.");
    const std::uint64_t seed = base_seed + seed_stride * static_cast<std::uint64_t>(index);
    print_row(scenario_labels[index], "baseline", sim::simulate(*scenario, baseline, trials, seed));
    print_row(scenario_labels[index], "pokemon-communication", sim::simulate(*scenario, communication, trials, seed));
  }
  return 0;
}

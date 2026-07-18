#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr std::uint64_t kTrials = 500000;
constexpr std::uint64_t kBaseSeed = 20260718;
constexpr std::uint64_t kSeedStride = 104729;

struct Stats {
  std::uint64_t trials{};
  std::array<std::uint64_t, 5> first_ready{};
  std::uint64_t not_ready_by_t4{};
  std::uint64_t started_regi{};
  std::uint64_t used_fss{};
  std::uint64_t used_legacy_star{};
  std::uint64_t used_steven{};
  std::uint64_t used_blender{};
  std::uint64_t total_mulligans{};
  long double ready_turn_sum{};
  long double ready_turn_square_sum{};
  long double restricted_turn_sum{};
  long double restricted_turn_square_sum{};
};

sim::DeckRecipe pokemon_communication_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto mawile = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == sim::Card::MawileGX;
  });
  if (mawile == recipe.end() || mawile->second != 1) {
    throw std::runtime_error(
        "The Pokémon Communication variant requires one baseline Mawile-GX.");
  }

  // Analyze the established one-card variant: Mawile-GX -1 and
  // Pokémon Communication +1.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Variant specification: https://github.com/FlareZ123/pokemon-sims/issues/701
  // Canonical regression: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/pokemon_communication_tests.cpp
  recipe.erase(mawilen        "The Pokémon Communication variant requires one baseline Mawile-GX.");
  }

  // Evaluate the established one-card variant: Mawile-GX -1 and Pokémon
  // Communication +1.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Variant specification: https://github.com/FlareZ123/pokemon-sims/issues/701
  // Canonical regression: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/pokemon_communication_tests.cpp
  recipe.erase(mawile);
  recipe.emplace_back(sim::Card::PokemonCommunication, 1);
  return recipe;
}

TurnStats simulate_turns(const sim::Scenario& scenario,
                         const sim::DeckRecipe& recipe,
                         const std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  TurnStats stats{};

  for (std::uint64_t trial = 0; trial < kTrials; ++trial) {
    sim::Engine engine(scenario, recipe, rng);
    const sim::TrialOutcome outcome = engine.run();
    ++stats.trials;

    const int ready_turn = outcome.first_ready_turn;
    if (ready_turn >= 2 && ready_turn <= 4) {
      ++stats.exact[static_cast<std::size_t>(ready_turn)];
      stats.ready_turn_sum += static_cast<std::uint64_t>(ready_turn);
      stats.ready_turn_square_sum +=
          static_cast<std::uint64_t>(ready_turn * ready_turn);
      stats.censored_turn_sum += static_cast<std::uint64_t>(ready_turn);
      stats.censored_turn_square_sum +=
          static_cast<std::uint64_t>(ready_turn * ready_turn);
    } else {
      ++stats.exact[0];
      // The engine ends at T4. Coding a miss as 5 gives E[min(T, 5)], a
      // conservative lower bound for eventual setup time.
      // Simulation horizon: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
      stats.censored_turn_sum += 5;
      stats.censored_turn_square_sum += 25;
    }

    stats.total_mulligans += outcome.mulligans;
    stats.started_regi += outcome.started_regi ? 1U : 0U;
    stats.used_fss += outcome.used_fss ? 1U : 0U;
    stats.used_legacy += outcome.used_legacy_star ? 1U : 0U;
    stats.used_steven += outcome.used_steven ? 1U : 0U;
    stats.used_blender += outcome.used_blender ? 1U : 0U;
  }

  return stats;
}

double mean(const std::uint64_t sum, const std::uint64_t count) {
  return count == 0 ? 0.0 :
      static_cast<double>(sum) / static_cast<double>(count);
}

double sample_mean_se(const std::uint64_t sum,
                      const std::uint64_t square_sum,
                      const std::uint64_t count) {
  if (count < 2) return 0.0;
  const double n = static_cast<double>(count);
  const double sum_value = static_cast<double>(sum);
  const double variance =
      (static_cast<double>(square_sum) - sum_value * sum_value / n) /
      (n - 1.0);
  return std::sqrt(std::max(0.0, variance) / n);
}

double pct(const std::uint64_t count, const std::uint64_t total) {
  return total == 0 ? 0.0 :
      100.0 * static_cast<double>(count) / static_cast<double>(total);
}

int quantile_code(const TurnStats& stats, const double quantile) {
  const double threshold = quantile * static_cast<double>(stats.trials);
  std::uint64_t cumulative = 0;
  for (int turn = 2; turn <= 4; ++turn) {
    cumulative += stats.exact[static_cast<std::size_t>(turn)];
    if (static_cast<double>(cumulative) >= threshold) return turn;
  }
  return 5;
}

void print_row(const std::string_view label, const TurnStats& stats) {
  const std::uint64_t ready_count =
      stats.exact[2] + stats.exact[3] + stats.exact[4];
  const std::uint64_t by2 = stats.exact[2];
  const std::uint64_t by3 = by2 + stats.exact[3];3] << ','
            << stats.first_ready[4] << ',' << stats.not_ready_by_t4 << ','
            << conditional_mean << ',' << conditional_se << ','
            << restricted_mean << ',' << restricted_se << ','
            << overall_quantile_turn(stats, 0.50L) << ','
            << overall_quantile_turn(stats, 0.75L) << ','
            << overall_quantile_turn(stats, 0.90L) << ','
            << static_cast<long double>(stats.total_mulligans) /
                   static_cast<long double>(stats.trials)
            << ',' << stats.started_regi << ',' << stats.used_fss << ','
            << stats.used_legacy_star << ',' << stats.used_steven << ','
            << stats.used_blender << '\n';
}

}  // namespace

int main() {
  constexpr std::array<std::string_view, 4> scenario_labels{{
      "matchup-flex-jit/go-first",
      "matchup-flex-jit/go-second",
      "no-discard-control/go-first",
      "no-discard-control/go-second",
  }};

  const sim::DeckRecipe recipe = pokemon_communication_recipe();
  std::cout << std::fixed << std::setprecision(9);
  std::cout
      << "scenario,trials,first_ready_t2,first_ready_t3,first_ready_t4,"
         "not_ready_by_t4,conditional_mean_ready_turn,conditional_mean_se,"
         ", stats.trials) << ','
            << pct(stats.used_fss, stats.trials) << ','
            << pct(stats.used_legacy, stats.trials) << ','
            << pct(stats.used_steven, stats.trials) << ','
            << pct(stats.used_blender, stats.trials) << '\n';
}

}  // namespace

int main() {
  constexpr std::array<std::string_view, 4> labels{{
      "matchup-flex-jit/go-first",
      "matchup-flex-jit/go-second",
      "no-discard-control/go-first",
      "no-discard-control/go-second",
  }};

  const sim::DeckRecipe recipe = pokemon_communication_recipe();

  // These are the repository's separate flex-JIT and no-discard-control profiles.
  // Profile definitions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
  // Probability contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#what-the-probability-means
  std::cout << std::fixed << std::setprecision(8);
  std::cout << "scenario,trials,t2_count,t3_count,t4_count,not_ready_t4_count,"
               "t2_exact_pct,t3_exact_pct,t4_exact_pct,not_ready_t4_pct,"
               "ready_by_t2_pct,ready_by_t3_pct,ready_by_t4_pct,"
               "mean_turn_given_ready_t4,mean_turn_given_ready_t4_se,"
               "turn5_censored_mean,turn5_censored_mean_se,"
               "median_code,p75_code,p90_code,avg_mulligans,"
               "started_regi_pct,used_fss_pct,used_legacy_pct,"
               "used_steven_pct,used_blender_pct\n";

  for (std::size_t index = 0; index < labels.size(); ++index) {
    const auto scenario = sim::scenario_by_label(std::string(labels[index]));
    if (!scenario) throw std::runtime_error("Unknown audit scenario.");
    print_row(labels[index], simulate_turns(
        *scenario, recipe,
        kBaseSeed + kSeedStride * static_cast<std::uint64_t>(index)));
  }

  return 0;
}

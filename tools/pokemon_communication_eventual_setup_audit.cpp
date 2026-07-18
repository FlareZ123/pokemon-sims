#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
constexpr std::uint64_t kTrials = 500000;
constexpr std::uint64_t kBaseSeed = 20260718;
constexpr std::uint64_t kSeedStride = 104729;
constexpr int kMaxTurns = 60;

struct EventualStats {
  std::uint64_t trials{};
  std::vector<std::uint64_t> first_ready;
  std::uint64_t never_ready{};
  long double successful_turn_sum{};
  long double successful_turn_square_sum{};
};

sim::DeckRecipe pokemon_communication_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto mawile = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == sim::Card::MawileGX;
  });
  if (mawile == recipe.end() || mawile->second != 1) {
    throw std::runtime_error("Expected the baseline Mawile-GX singleton.");
  }
  // Established variant: Mawile-GX -1, Pokemon Communication +1.
  // Pokemon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Variant contract: https://github.com/FlareZ123/pokemon-sims/issues/701
  recipe.erase(mawile);
  recipe.emplace_back(sim::Card::PokemonCommunication, 1);
  return recipe;
}

EventualStats simulate_eventual(sim::Scenario scenario,
                                const sim::DeckRecipe& recipe,
                                const std::uint64_t seed) {
  // Continue beyond the repository reporting horizon until readiness or deck-out.
  // Engine loop: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
  // Deck-out terminal: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L12-L17
  // Readiness records any turn: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L177
  scenario.max_turn = kMaxTurns;
  std::mt19937_64 rng(seed);
  EventualStats stats{};
  stats.first_ready.assign(static_cast<std::size_t>(kMaxTurns + 1), 0);

  for (std::uint64_t trial = 0; trial < kTrials; ++trial) {
    sim::Engine engine(scenario, recipe, rng);
    const sim::TrialOutcome outcome = engine.run();
    ++stats.trials;
    if (outcome.first_ready_turn >= 2 && outcome.first_ready_turn <= kMaxTurns) {
      const auto index = static_cast<std::size_t>(outcome.first_ready_turn);
      ++stats.first_ready[index];
      const long double turn = static_cast<long double>(outcome.first_ready_turn);
      stats.successful_turn_sum += turn;
      stats.successful_turn_square_sum += turn * turn;
    } else {
      ++stats.never_ready;
    }
  }
  return stats;
}

long double mean(const long double sum, const std::uint64_t count) {
  return count == 0 ? 0.0L : sum / static_cast<long double>(count);
}

long double sample_sd(const long double sum, const long double square_sum,
                      const std::uint64_t count) {
  if (count < 2) return 0.0L;
  const long double n = static_cast<long double>(count);
  return std::sqrt(std::max(0.0L, (square_sum - sum * sum / n) / (n - 1.0L)));
}

int quantile_turn(const EventualStats& stats, const long double q) {
  const std::uint64_t successes = stats.trials - stats.never_ready;
  const long double target = q * static_cast<long double>(successes);
  std::uint64_t cumulative = 0;
  for (int turn = 2; turn <= kMaxTurns; ++turn) {
    cumulative += stats.first_ready[static_cast<std::size_t>(turn)];
    if (static_cast<long double>(cumulative) >= target) return turn;
  }
  return 0;
}

void print_summary(const std::string_view label, const EventualStats& stats) {
  const std::uint64_t successes = stats.trials - stats.never_ready;
  std::cout << "SUMMARY," << label << ',' << stats.trials << ',' << successes << ','
            << stats.never_ready << ','
            << mean(stats.successful_turn_sum, successes) << ','
            << sample_sd(stats.successful_turn_sum, stats.successful_turn_square_sum, successes)
            << ',' << quantile_turn(stats, 0.50L)
            << ',' << quantile_turn(stats, 0.75L)
            << ',' << quantile_turn(stats, 0.90L)
            << ',' << quantile_turn(stats, 0.95L)
            << ',' << quantile_turn(stats, 0.99L) << '\n';
  for (int turn = 2; turn <= kMaxTurns; ++turn) {
    const std::uint64_t count = stats.first_ready[static_cast<std::size_t>(turn)];
    if (count != 0) std::cout << "TURN," << label << ',' << turn << ',' << count << '\n';
  }
  std::cout << "NEVER," << label << ",0," << stats.never_ready << '\n';
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
  std::cout << std::fixed << std::setprecision(9);
  std::cout << "record,scenario,trials_or_turn,successes_or_count,never_ready,mean_success_turn,sd_success_turn,p50,p75,p90,p95,p99\n";
  for (std::size_t index = 0; index < labels.size(); ++index) {
    const auto scenario = sim::scenario_by_label(std::string(labels[index]));
    if (!scenario) throw std::runtime_error("Unknown scenario.");
    print_summary(labels[index], simulate_eventual(
        *scenario, recipe,
        kBaseSeed + kSeedStride * static_cast<std::uint64_t>(index)));
  }
  return 0;
}

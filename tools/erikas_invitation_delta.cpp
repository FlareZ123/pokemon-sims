#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
constexpr std::uint64_t kTrials = 200000;
constexpr std::uint64_t kBaseSeed = 2026071801ULL;
constexpr std::array<int, 3> kHorizons{{4, 5, 60}};

struct Metrics {
  std::uint64_t trials{};
  std::uint64_t ready_by_2{};
  std::uint64_t ready_by_3{};
  std::uint64_t ready_by_4{};
  std::uint64_t ready_by_5{};
  std::uint64_t success{};
  std::uint64_t never_ready{};
  long double turn_sum{};
  long double mulligan_sum{};
  std::uint64_t started_regi{};
};

std::uint64_t splitmix64(std::uint64_t value) {
  value += 0x9e3779b97f4a7c15ULL;
  value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31U);
}

int recipe_size(const sim::DeckRecipe& recipe) {
  return std::accumulate(recipe.begin(), recipe.end(), 0,
                         [](const int total, const auto& entry) {
                           return total + entry.second;
                         });
}

int copies_of(const sim::DeckRecipe& recipe, const sim::Card card) {
  for (const auto& [candidate, copies] : recipe) {
    if (candidate == card) return copies;
  }
  return 0;
}

sim::DeckRecipe mawile_control_recipe(const sim::DeckRecipe& erika_recipe) {
  sim::DeckRecipe recipe = erika_recipe;
  bool replaced = false;
  for (auto& [card, copies] : recipe) {
    if (card == sim::Card::ErikasInvitation) {
      if (copies != 1 || replaced) {
        throw std::runtime_error("Expected one Erika's Invitation singleton.");
      }
      card = sim::Card::MawileGX;
      replaced = true;
    }
  }
  if (!replaced) throw std::runtime_error("Erika's Invitation was absent from the main recipe.");
  return recipe;
}

void observe(Metrics& metrics, const sim::TrialOutcome& outcome) {
  ++metrics.trials;
  metrics.mulligan_sum += static_cast<long double>(outcome.mulligans);
  if (outcome.started_regi) ++metrics.started_regi;
  const int turn = outcome.first_ready_turn;
  if (turn > 0) {
    ++metrics.success;
    metrics.turn_sum += static_cast<long double>(turn);
    if (turn <= 2) ++metrics.ready_by_2;
    if (turn <= 3) ++metrics.ready_by_3;
    if (turn <= 4) ++metrics.ready_by_4;
    if (turn <= 5) ++metrics.ready_by_5;
  } else {
    ++metrics.never_ready;
  }
}

std::array<Metrics, 2> simulate_pair(sim::Scenario scenario,
                                     const sim::DeckRecipe& mawile_recipe,
                                     const sim::DeckRecipe& erika_recipe,
                                     const int horizon,
                                     const std::uint64_t seed_namespace) {
  scenario.max_turn = horizon;
  std::array<Metrics, 2> result{};
  for (std::uint64_t trial = 0; trial < kTrials; ++trial) {
    const std::uint64_t seed = splitmix64(seed_namespace + trial);
    for (std::size_t deck_index = 0; deck_index < result.size(); ++deck_index) {
      std::mt19937_64 rng(seed);
      const sim::DeckRecipe& recipe = deck_index == 0 ? mawile_recipe : erika_recipe;
      sim::Engine engine(scenario, recipe, rng);
      observe(result[deck_index], engine.run());
    }
  }
  return result;
}

long double pct(const std::uint64_t count, const std::uint64_t trials) {
  return trials == 0 ? 0.0L : 100.0L * static_cast<long double>(count) /
                                  static_cast<long double>(trials);
}

long double mean(const long double sum, const std::uint64_t count) {
  return count == 0 ? 0.0L : sum / static_cast<long double>(count);
}

void print_raw(const std::string_view scenario, const int horizon,
               const std::string_view deck, const Metrics& metrics) {
  std::cout << "RAW," << scenario << ',' << horizon << ',' << deck << ','
            << metrics.trials << ','
            << pct(metrics.ready_by_2, metrics.trials) << ','
            << pct(metrics.ready_by_3, metrics.trials) << ','
            << pct(metrics.ready_by_4, metrics.trials) << ','
            << pct(metrics.ready_by_5, metrics.trials) << ','
            << pct(metrics.success, metrics.trials) << ','
            << pct(metrics.never_ready, metrics.trials) << ','
            << mean(metrics.turn_sum, metrics.success) << ','
            << mean(metrics.mulligan_sum, metrics.trials) << ','
            << pct(metrics.started_regi, metrics.trials) << '\n';
}

void print_delta(const std::string_view scenario, const int horizon,
                 const Metrics& mawile, const Metrics& erika) {
  std::cout << "DELTA," << scenario << ',' << horizon << ",erika-minus-mawile,"
            << erika.trials << ','
            << pct(erika.ready_by_2, erika.trials) - pct(mawile.ready_by_2, mawile.trials) << ','
            << pct(erika.ready_by_3, erika.trials) - pct(mawile.ready_by_3, mawile.trials) << ','
            << pct(erika.ready_by_4, erika.trials) - pct(mawile.ready_by_4, mawile.trials) << ','
            << pct(erika.ready_by_5, erika.trials) - pct(mawile.ready_by_5, mawile.trials) << ','
            << pct(erika.success, erika.trials) - pct(mawile.success, mawile.trials) << ','
            << pct(erika.never_ready, erika.trials) - pct(mawile.never_ready, mawile.trials) << ','
            << mean(erika.turn_sum, erika.success) - mean(mawile.turn_sum, mawile.success) << ','
            << mean(erika.mulligan_sum, erika.trials) - mean(mawile.mulligan_sum, mawile.trials) << ','
            << pct(erika.started_regi, erika.trials) - pct(mawile.started_regi, mawile.trials) << '\n';
}
}  // namespace

int main() {
  const sim::DeckRecipe erika_recipe = sim::baseline_recipe();
  const sim::DeckRecipe mawile_recipe = mawile_control_recipe(erika_recipe);
  if (recipe_size(erika_recipe) != 60 || recipe_size(mawile_recipe) != 60 ||
      copies_of(erika_recipe, sim::Card::ErikasInvitation) != 1 ||
      copies_of(erika_recipe, sim::Card::MawileGX) != 0 ||
      copies_of(mawile_recipe, sim::Card::ErikasInvitation) != 0 ||
      copies_of(mawile_recipe, sim::Card::MawileGX) != 1 ||
      !sim::is_supporter(sim::Card::ErikasInvitation) ||
      sim::is_basic(sim::Card::ErikasInvitation) ||
      sim::is_pokemon(sim::Card::ErikasInvitation)) {
    throw std::runtime_error("Erika/Mawile recipe or type contract failed.");
  }

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "record,scenario,horizon,deck_or_delta,trials,ready_by_2_pct,ready_by_3_pct,ready_by_4_pct,ready_by_5_pct,success_by_horizon_pct,never_by_horizon_pct,mean_success_turn,avg_mulligans,started_regi_pct\n";

  std::size_t scenario_index = 0;
  for (const sim::Scenario& scenario : sim::all_scenarios()) {
    if (scenario.locks != sim::LockMode::None) continue;
    for (const int horizon : kHorizons) {
      const std::uint64_t seed_namespace = kBaseSeed +
          100000000ULL * static_cast<std::uint64_t>(scenario_index) +
          1000000ULL * static_cast<std::uint64_t>(horizon);
      const auto pair = simulate_pair(scenario, mawile_recipe, erika_recipe,
                                      horizon, seed_namespace);
      print_raw(scenario.label, horizon, "mawile-control", pair[0]);
      print_raw(scenario.label, horizon, "erikas-invitation", pair[1]);
      print_delta(scenario.label, horizon, pair[0], pair[1]);
    }
    ++scenario_index;
  }
  return 0;
}

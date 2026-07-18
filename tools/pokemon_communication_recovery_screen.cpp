#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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

namespace sim {
struct EngineTestAccess {
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {
constexpr std::uint64_t kTrialsPerSeat = 100000;
constexpr std::uint64_t kBaseSeed = 20260718;
constexpr int kRecoveryDeadline = 5;
constexpr int kMaxTurns = 60;

struct Candidate {
  sim::Card card;
  std::string_view label;
};

constexpr std::array<Candidate, 29> kCandidates{{
    {sim::Card::QuickBall, "Quick Ball"},
    {sim::Card::PokemonCommunication, "Pokemon Communication"},
    {sim::Card::UltraBall, "Ultra Ball"},
    {sim::Card::EvolutionIncense, "Evolution Incense"},
    {sim::Card::EarthenVessel, "Earthen Vessel"},
    {sim::Card::Arven, "Arven"},
    {sim::Card::Crispin, "Crispin"},
    {sim::Card::ProfessorBurnet, "Professor Burnet"},
    {sim::Card::Serena, "Serena"},
    {sim::Card::TateLiza, "Tate & Liza"},
    {sim::Card::StevensResolve, "Steven's Resolve"},
    {sim::Card::Gladion, "Gladion"},
    {sim::Card::ForestSealStone, "Forest Seal Stone"},
    {sim::Card::Powerglass, "Powerglass"},
    {sim::Card::Grass, "Grass Energy"},
    {sim::Card::Fire, "Fire Energy"},
    {sim::Card::RegidragoVstar, "Regidrago VSTAR"},
    {sim::Card::TapuLeleGX, "Tapu Lele-GX"},
    {sim::Card::HisuianHeavyBall, "Hisuian Heavy Ball"},
    {sim::Card::Oricorio, "Oricorio"},
    {sim::Card::Dragapult, "Dragapult ex"},
    {sim::Card::MegaDragonite, "Mega Dragonite ex"},
    {sim::Card::DialgaGX, "Dialga-GX"},
    {sim::Card::GoodraVstar, "Hisuian Goodra VSTAR"},
    {sim::Card::Lusamine, "Lusamine"},
    {sim::Card::TeamYellsCheer, "Team Yell's Cheer"},
    {sim::Card::RoseannesBackup, "Roseanne's Backup"},
    {sim::Card::ProfessorTuro, "Professor Turo's Scenario"},
    {sim::Card::Dipplin, "Dipplin"},
}};

struct Metrics {
  std::uint64_t trials{};
  std::array<std::uint64_t, kMaxTurns + 1> first_ready{};
  std::uint64_t never_ready{};
  long double successful_turn_sum{};
  long double successful_turn_square_sum{};
  std::array<std::uint64_t, 8> t5_missing_mask{};
  std::array<std::uint64_t, 8> never_missing_mask{};
  std::uint64_t never_all_vstar_prized{};
  std::uint64_t never_all_fire_prized{};
  std::uint64_t never_all_grass_prized{};
  std::uint64_t never_all_payload_prized{};
};

std::uint64_t splitmix64(std::uint64_t value) {
  value += 0x9e3779b97f4a7c15ULL;
  value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31U);
}

int recipe_count(const sim::DeckRecipe& recipe, const sim::Card card) {
  const auto it = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  return it == recipe.end() ? 0 : it->second;
}

void change_count(sim::DeckRecipe& recipe, const sim::Card card, const int delta) {
  const auto it = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
    return entry.first == card;
  });
  if (it == recipe.end()) {
    if (delta <= 0) throw std::runtime_error("Cannot remove an absent card.");
    recipe.emplace_back(card, delta);
    return;
  }
  it->second += delta;
  if (it->second < 0) throw std::runtime_error("Negative card count.");
  if (it->second == 0) recipe.erase(it);
}

int total_cards(const sim::DeckRecipe& recipe) {
  return std::accumulate(recipe.begin(), recipe.end(), 0,
                         [](const int sum, const auto& entry) { return sum + entry.second; });
}

sim::DeckRecipe pokemon_communication_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  change_count(recipe, sim::Card::MawileGX, -1);
  change_count(recipe, sim::Card::PokemonCommunication, 1);
  if (total_cards(recipe) != 60) throw std::runtime_error("Variant must contain 60 cards.");
  return recipe;
}

sim::DeckRecipe replacement_recipe(const sim::Card add) {
  sim::DeckRecipe recipe = pokemon_communication_recipe();
  change_count(recipe, sim::Card::PathToPeak, -1);
  change_count(recipe, add, 1);
  if (total_cards(recipe) != 60) throw std::runtime_error("Replacement must contain 60 cards.");
  if (add != sim::Card::Grass && add != sim::Card::Fire && recipe_count(recipe, add) > 4) {
    throw std::runtime_error("Candidate exceeds the four-copy limit.");
  }
  return recipe;
}

int missing_mask(const sim::Engine& engine) {
  const sim::State& state = engine.state();
  const bool active_vstar = state.active && state.active->card == sim::Card::RegidragoVstar;
  const bool energy_ready = active_vstar && state.active->grass >= 2 && state.active->fire >= 1;
  const bool payload_ready = sim::EngineTestAccess::payload_ready(engine);
  int mask = 0;
  if (!active_vstar) mask |= 1;
  if (!energy_ready) mask |= 2;
  if (!payload_ready) mask |= 4;
  return mask;
}

int zone_count(const std::vector<sim::Card>& zone, const sim::Card card) {
  return static_cast<int>(std::count(zone.begin(), zone.end(), card));
}

bool all_copies_prized(const sim::DeckRecipe& recipe, const std::vector<sim::Card>& prizes,
                       const sim::Card card) {
  const int copies = recipe_count(recipe, card);
  return copies > 0 && zone_count(prizes, card) == copies;
}

bool all_payloads_prized(const sim::DeckRecipe& recipe, const std::vector<sim::Card>& prizes) {
  constexpr std::array<sim::Card, 4> payloads{{
      sim::Card::Dragapult, sim::Card::MegaDragonite,
      sim::Card::DialgaGX, sim::Card::GoodraVstar,
  }};
  int total = 0;
  int prized = 0;
  for (const sim::Card card : payloads) {
    total += recipe_count(recipe, card);
    prized += zone_count(prizes, card);
  }
  return total > 0 && prized == total;
}

void add_metrics(Metrics& target, const Metrics& source) {
  target.trials += source.trials;
  target.never_ready += source.never_ready;
  target.successful_turn_sum += source.successful_turn_sum;
  target.successful_turn_square_sum += source.successful_turn_square_sum;
  target.never_all_vstar_prized += source.never_all_vstar_prized;
  target.never_all_fire_prized += source.never_all_fire_prized;
  target.never_all_grass_prized += source.never_all_grass_prized;
  target.never_all_payload_prized += source.never_all_payload_prized;
  for (std::size_t i = 0; i < target.first_ready.size(); ++i) {
    target.first_ready[i] += source.first_ready[i];
  }
  for (std::size_t i = 0; i < target.t5_missing_mask.size(); ++i) {
    target.t5_missing_mask[i] += source.t5_missing_mask[i];
    target.never_missing_mask[i] += source.never_missing_mask[i];
  }
}

Metrics simulate_seat(sim::Scenario scenario, const sim::DeckRecipe& recipe,
                      const std::uint64_t seat_seed) {
  Metrics metrics{};
  sim::Scenario deadline_scenario = scenario;
  deadline_scenario.max_turn = kRecoveryDeadline;
  scenario.max_turn = kMaxTurns;

  for (std::uint64_t trial = 0; trial < kTrialsPerSeat; ++trial) {
    const std::uint64_t trial_seed = splitmix64(seat_seed + trial);

    std::mt19937_64 deadline_rng(trial_seed);
    sim::Engine deadline_engine(deadline_scenario, recipe, deadline_rng);
    const sim::TrialOutcome deadline_outcome = deadline_engine.run();
    if (deadline_outcome.first_ready_turn == 0) {
      ++metrics.t5_missing_mask[static_cast<std::size_t>(missing_mask(deadline_engine))];
    }

    std::mt19937_64 eventual_rng(trial_seed);
    sim::Engine eventual_engine(scenario, recipe, eventual_rng);
    const sim::TrialOutcome eventual_outcome = eventual_engine.run();
    ++metrics.trials;
    if (eventual_outcome.first_ready_turn >= 2 && eventual_outcome.first_ready_turn <= kMaxTurns) {
      const int turn = eventual_outcome.first_ready_turn;
      ++metrics.first_ready[static_cast<std::size_t>(turn)];
      const long double value = static_cast<long double>(turn);
      metrics.successful_turn_sum += value;
      metrics.successful_turn_square_sum += value * value;
    } else {
      ++metrics.never_ready;
      ++metrics.never_missing_mask[static_cast<std::size_t>(missing_mask(eventual_engine))];
      const auto& prizes = eventual_engine.state().prizes;
      if (all_copies_prized(recipe, prizes, sim::Card::RegidragoVstar)) {
        ++metrics.never_all_vstar_prized;
      }
      if (all_copies_prized(recipe, prizes, sim::Card::Fire)) ++metrics.never_all_fire_prized;
      if (all_copies_prized(recipe, prizes, sim::Card::Grass)) ++metrics.never_all_grass_prized;
      if (all_payloads_prized(recipe, prizes)) ++metrics.never_all_payload_prized;
    }
  }
  return metrics;
}

std::uint64_t ready_by(const Metrics& metrics, const int deadline) {
  std::uint64_t result = 0;
  for (int turn = 2; turn <= deadline; ++turn) {
    result += metrics.first_ready[static_cast<std::size_t>(turn)];
  }
  return result;
}

int quantile_turn(const Metrics& metrics, const long double quantile) {
  const std::uint64_t successes = metrics.trials - metrics.never_ready;
  const long double target = quantile * static_cast<long double>(successes);
  std::uint64_t cumulative = 0;
  for (int turn = 2; turn <= kMaxTurns; ++turn) {
    cumulative += metrics.first_ready[static_cast<std::size_t>(turn)];
    if (static_cast<long double>(cumulative) >= target) return turn;
  }
  return 0;
}

long double mean_success_turn(const Metrics& metrics) {
  const std::uint64_t successes = metrics.trials - metrics.never_ready;
  return successes == 0 ? 0.0L :
      metrics.successful_turn_sum / static_cast<long double>(successes);
}

Metrics simulate_both_seats(const sim::DeckRecipe& recipe) {
  Metrics aggregate{};
  constexpr std::array<std::string_view, 2> labels{{
      "no-discard-control/go-first", "no-discard-control/go-second",
  }};
  for (std::size_t index = 0; index < labels.size(); ++index) {
    const auto scenario = sim::scenario_by_label(std::string(labels[index]));
    if (!scenario) throw std::runtime_error("Unknown scenario.");
    add_metrics(aggregate, simulate_seat(
        *scenario, recipe, kBaseSeed + 1000003ULL * static_cast<std::uint64_t>(index)));
  }
  return aggregate;
}

void print_metrics(const std::string_view variant, const std::string_view add_card,
                   const Metrics& metrics) {
  const std::uint64_t successes = metrics.trials - metrics.never_ready;
  std::cout << variant << ',' << add_card << ",Path to the Peak," << metrics.trials << ','
            << ready_by(metrics, 2) << ',' << ready_by(metrics, 3) << ','
            << ready_by(metrics, 4) << ',' << ready_by(metrics, 5) << ','
            << metrics.trials - ready_by(metrics, 5) << ',' << successes << ','
            << metrics.never_ready << ',' << mean_success_turn(metrics) << ','
            << quantile_turn(metrics, 0.50L) << ',' << quantile_turn(metrics, 0.75L) << ','
            << quantile_turn(metrics, 0.90L) << ',' << quantile_turn(metrics, 0.95L);
  for (std::size_t mask = 1; mask < metrics.t5_missing_mask.size(); ++mask) {
    std::cout << ',' << metrics.t5_missing_mask[mask];
  }
  for (std::size_t mask = 1; mask < metrics.never_missing_mask.size(); ++mask) {
    std::cout << ',' << metrics.never_missing_mask[mask];
  }
  std::cout << ',' << metrics.never_all_vstar_prized
            << ',' << metrics.never_all_fire_prized
            << ',' << metrics.never_all_grass_prized
            << ',' << metrics.never_all_payload_prized << '\n';
}
}  // namespace

int main() {
  std::cout << std::fixed << std::setprecision(9);
  std::cout << "variant,add_card,cut_card,trials,ready_by_t2,ready_by_t3,ready_by_t4,ready_by_t5,after_t5_or_never,eventual_success,never_ready,mean_success_turn,p50,p75,p90,p95"
               ",t5_missing_active,t5_missing_energy,t5_missing_active_energy,t5_missing_payload,t5_missing_active_payload,t5_missing_energy_payload,t5_missing_all"
               ",never_missing_active,never_missing_energy,never_missing_active_energy,never_missing_payload,never_missing_active_payload,never_missing_energy_payload,never_missing_all"
               ",never_all_vstar_prized,never_all_fire_prized,never_all_grass_prized,never_all_payload_prized\n";

  print_metrics("baseline", "none", simulate_both_seats(pokemon_communication_recipe()));
  for (const Candidate& candidate : kCandidates) {
    print_metrics("path-cut-screen", candidate.label,
                  simulate_both_seats(replacement_recipe(candidate.card)));
  }
  return 0;
}

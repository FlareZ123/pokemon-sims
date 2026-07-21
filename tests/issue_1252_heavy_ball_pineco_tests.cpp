#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool needs_pineco_connector(const Engine& engine) {
    return engine.needs_pineco_connector();
  }
};
}

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_prized_pineco_is_recovered_for_live_connector() {
  const sim::Scenario scenario{"issue-1252", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1252};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::Pineco, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::Powerglass, sim::Card::FieldBlower};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Hisuian Heavy Ball may recover a revealed Basic Pokémon from the Prize cards.
  // Pineco is a Basic and is the missing live connector for Forretress ex:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1252
  if (!sim::EngineTestAccess::needs_pineco_connector(engine) ||
      !sim::EngineTestAccess::play_heavy_ball(engine)) {
    throw std::runtime_error("The live prized-Pineco Heavy Ball route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Pineco) ||
      contains(after.prizes, sim::Card::Pineco) ||
      !contains(after.prizes, sim::Card::HisuianHeavyBall) ||
      contains(after.discard, sim::Card::HisuianHeavyBall)) {
    throw std::runtime_error("Hisuian Heavy Ball did not exchange for prized Pineco.");
  }
}
}

int main() {
  try {
    test_prized_pineco_is_recovered_for_live_connector();
    std::cout << "Issue 1252 Hisuian Heavy Ball tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

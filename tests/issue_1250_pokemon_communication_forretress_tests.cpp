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
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool needs_pineco_connector(const Engine& engine) {
    return engine.needs_pineco_connector();
  }
  static bool needs_forretress_connector(const Engine& engine) {
    return engine.needs_forretress_connector();
  }
  static bool run_forretress_combo_search_step(Engine& engine) {
    return engine.run_forretress_combo_search_step(false);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::DeckRecipe variant_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::PokemonCommunication, 1);
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  recipe.emplace_back(sim::Card::Appletun, 1);
  return recipe;
}

void expect_exchange(const bool pineco_target) {
  const sim::Scenario scenario{pineco_target ? "issue-1250-pineco" : "issue-1250-forretress",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{pineco_target ? 1250U : 12501U};
  const sim::DeckRecipe recipe = variant_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  if (!pineco_target) state.bench = {sim::Pokemon{sim::Card::Pineco, 1}};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Appletun,
                sim::Card::RegidragoVstar};
  const sim::Card target = pineco_target ? sim::Card::Pineco : sim::Card::ForretressEx;
  state.deck = {target, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Pokémon Communication accepts any Pokémon from hand and searches any
  // Pokémon. Appletun is a legal exchange while the missing Pineco line is the
  // live connector axis; the singleton held Regidrago VSTAR remains protected:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1250
  const bool axis_live = pineco_target
      ? sim::EngineTestAccess::needs_pineco_connector(engine)
      : sim::EngineTestAccess::needs_forretress_connector(engine);
  if (!axis_live || !sim::EngineTestAccess::run_forretress_combo_search_step(engine)) {
    throw std::runtime_error("Pokémon Communication did not complete the live Forretress connector route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, target) || contains(after.hand, sim::Card::Appletun) ||
      !contains(after.deck, sim::Card::Appletun) ||
      !contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.discard, sim::Card::PokemonCommunication)) {
    throw std::runtime_error("The Appletun exchange resolved incorrectly.");
  }
}

}  // namespace

int main() {
  try {
    expect_exchange(true);
    expect_exchange(false);
    std::cout << "Issue 1250 Pokémon Communication tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

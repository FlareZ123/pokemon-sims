#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
namespace sim { struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play(Engine& engine) { return engine.play_mysterious_treasure(true); }
}; }
namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}
void test_preferred_appletun_payload() {
  const sim::Scenario scenario{"issue-1253", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1253};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Appletun, 1);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Grant,
                sim::Card::Serena, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Appletun, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Mysterious Treasure searches a Psychic or Dragon Pokémon after discarding one
  // card. Appletun sv8-140 is Dragon and a modeled Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1253
  if (!sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error("Mysterious Treasure did not play.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::MysteriousTreasure) ||
      !contains(after.discard, sim::Card::Grant)) {
    throw std::runtime_error("Mysterious Treasure failed to fetch Appletun.");
  }
}
}
int main() {
  try {
    test_preferred_appletun_payload();
    std::cout << "Issue 1253 Mysterious Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

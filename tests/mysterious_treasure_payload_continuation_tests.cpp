#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine) { return engine.play_mysterious_treasure(true); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_two_mysterious_treasures_do_not_strand_a_fetched_payload() {
  // Mysterious Treasure must discard a card before it searches for a Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // Apex Dragon needs a Dragon Pokémon attack in discard, so a fetched payload must
  // still have a legal same-turn discard continuation: https://api.pokemontcg.io/v2/cards/swsh12-136
  sim::Scenario scenario{"two-treasure-payload-continuation", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  // The Supporter play has already been used, so Tapu Lele-GX cannot open a
  // separate Supporter-search axis in this payload-only fixture:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  state.supporter_used = true;

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{140};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "The second Mysterious Treasure is the only current cost and cannot also be the payload outlet.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 2U && contains(result.hand, sim::Card::MysteriousTreasure),
         "The policy should retain both Mysterious Treasures rather than strand a fetched payload.");
  expect(contains(result.deck, sim::Card::MegaDragonite),
         "The Dragon payload should remain in deck until a live discard continuation exists.");
}

}  // namespace

int main() {
  try {
    test_two_mysterious_treasures_do_not_strand_a_fetched_payload();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

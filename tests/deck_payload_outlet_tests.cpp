#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool value) { engine.deck_seen_ = value; }
  static bool play_brilliant_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool play_professor_burnet(Engine& engine) { return engine.play_professor_burnet(); }
};

}  // namespace sim

namespace {

using sim::Card;
using sim::DciProfile;
using sim::Engine;
using sim::EngineTestAccess;
using sim::LockMode;
using sim::Pokemon;
using sim::Scenario;
using sim::State;
using sim::Tool;

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<Card>& cards, const Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

State known_empty_payload_deck_state(const Card outlet) {
  State result;
  result.turn = 2;
  result.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  result.hand = {outlet, Card::MegaDragonite};
  result.deck = {Card::Dipplin, Card::Grass, Card::Fire};
  return result;
}

void test_blender_is_held_when_k1_proves_no_payload_in_deck() {
  const Scenario scenario{"k1-blender", DciProfile::StrictJit, LockMode::None, false, 4};
  std::mt19937_64 rng{1};
  Engine engine(scenario, sim::baseline_recipe(), rng);
  EngineTestAccess::set_state(engine, known_empty_payload_deck_state(Card::BrilliantBlender));
  EngineTestAccess::set_deck_seen(engine, true);

  expect(!EngineTestAccess::play_brilliant_blender(engine),
         "K1 must hold Brilliant Blender when no payload remains in the deck.");
  const State& after = engine.state();
  expect(contains(after.hand, Card::BrilliantBlender),
         "Holding a known-dead Blender must preserve it in hand.");
  expect(!contains(after.discard, Card::BrilliantBlender),
         "Holding a known-dead Blender must not discard it.");
}

void test_burnet_is_held_when_k1_proves_no_payload_in_deck() {
  const Scenario scenario{"k1-burnet", DciProfile::StrictJit, LockMode::None, false, 4};
  std::mt19937_64 rng{2};
  Engine engine(scenario, sim::baseline_recipe(), rng);
  EngineTestAccess::set_state(engine, known_empty_payload_deck_state(Card::ProfessorBurnet));
  EngineTestAccess::set_deck_seen(engine, true);

  expect(!EngineTestAccess::play_professor_burnet(engine),
         "K1 must hold Professor Burnet when no payload remains in the deck.");
  const State& after = engine.state();
  expect(contains(after.hand, Card::ProfessorBurnet),
         "Holding a known-dead Burnet must preserve it in hand.");
  expect(!contains(after.discard, Card::ProfessorBurnet),
         "Holding a known-dead Burnet must not spend the Supporter slot.");
  expect(!after.supporter_used, "Holding a known-dead Burnet must preserve the Supporter slot.");
}

}  // namespace

int main() {
  try {
    test_blender_is_held_when_k1_proves_no_payload_in_deck();
    test_burnet_is_held_when_k1_proves_no_payload_in_deck();
    std::cout << "deck payload outlet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

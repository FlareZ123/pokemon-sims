#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_known_prizes_do_not_preempt_serena_payload_under_item_lock() {
  // Serena's draw mode requires at least one discard; the payload is that cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // The player may play one Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"gladion-serena-item-lock", sim::DciProfile::StrictJit, sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{59};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Serena, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Lusamine, sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin,
                  sim::Card::MawileGX, sim::Card::Guzma};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Serena must use the one available Supporter play");
  expect(contains(after.hand, sim::Card::Gladion), "Gladion must be preserved when Serena directly completes payload timing");
  expect(contains(after.discard, sim::Card::Serena), "Serena must enter discard after being played");
  expect(contains(after.discard, sim::Card::MegaDragonite), "Serena must discard the modeled Dragon payload");
  expect(contains(after.discarded_this_turn, sim::Card::MegaDragonite), "the payload must be marked as discarded this turn");
  expect(sim::EngineTestAccess::payload_ready(engine), "the Serena discard must satisfy strict current-turn payload timing");
}

}  // namespace

int main() {
  try {
    test_known_prizes_do_not_preempt_serena_payload_under_item_lock();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_roseanne_restores_missing_fire_for_same_turn_vessel_attachment() {
  const sim::Scenario scenario{"roseanne-energy-vessel", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{305};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::EarthenVessel, sim::Card::Dipplin};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Roseanne's Backup may shuffle an Energy from discard into the deck. Earthen
  // Vessel can then search that Basic Energy, and the unused once-per-turn manual
  // attachment can put it on the Active Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar && after.active->fire == 1,
         "The restored Fire Energy should be searched and manually attached in the same turn");
  expect(after.supporter_used && after.manual_energy_used,
         "The Roseanne Supporter play and manual Energy attachment should both be consumed");
  expect(contains(after.discard, sim::Card::RoseannesBackup) &&
             contains(after.discard, sim::Card::EarthenVessel) && contains(after.discard, sim::Card::Dipplin),
         "Roseanne, Earthen Vessel, and the Vessel cost should be in discard");
  expect(!contains(after.deck, sim::Card::Fire) && !contains(after.hand, sim::Card::Fire),
         "The restored Fire Energy should finish attached rather than remain in deck or hand");
}

}  // namespace

int main() {
  try {
    test_roseanne_restores_missing_fire_for_same_turn_vessel_attachment();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

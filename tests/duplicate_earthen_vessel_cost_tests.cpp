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
  static bool play_earthen_vessel(Engine& engine, const bool permit_payload) {
    return engine.play_earthen_vessel(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_duplicate_earthen_vessel_is_legal_cost() {
  const sim::Scenario scenario{"duplicate-earthen-vessel-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{86};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  // Earthen Vessel requires discarding another card from hand before searching
  // Basic Energy, so the other copy is a legal cost: https://api.pokemontcg.io/v2/cards/sv4-163
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_earthen_vessel(engine, false),
         "Earthen Vessel should use the other copy as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Grass) && contains(after.hand, sim::Card::Fire),
         "Earthen Vessel should search both needed Basic Energy cards");
  expect(count(after.discard, sim::Card::EarthenVessel) == 2,
         "the played Earthen Vessel and its duplicate cost should both be discarded");
  expect(after.deck.empty(), "the searched Basic Energy cards should leave the deck");
}

void test_earthen_vessel_can_discard_unpayable_ultra_ball() {
  const sim::Scenario scenario{"earthen-vessel-ultra-ball-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{87};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::UltraBall};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  // Earthen Vessel needs one other hand card. Ultra Ball needs two other cards only
  // when Ultra Ball itself is played, so it remains a legal Vessel cost here:
  // https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_earthen_vessel(engine, false),
         "Earthen Vessel should use held Ultra Ball as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Grass) && contains(after.hand, sim::Card::Fire),
         "Earthen Vessel should search both needed Basic Energy cards");
  expect(count(after.discard, sim::Card::EarthenVessel) == 1,
         "the played Earthen Vessel should be discarded");
  expect(count(after.discard, sim::Card::UltraBall) == 1,
         "the held Ultra Ball should be discarded as the cost");
  expect(after.deck.empty(), "the searched Basic Energy cards should leave the deck");
}

}  // namespace

int main() {
  try {
    test_duplicate_earthen_vessel_is_legal_cost();
    test_earthen_vessel_can_discard_unpayable_ultra_ball();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool hold_vessel(const Engine& engine) {
    return engine.hold_earthen_vessel_for_next_turn_latias_jit();
  }
  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  Fixture()
      : scenario{"issue-1471", sim::DciProfile::StrictJit,
                 sim::LockMode::None, true, 4},
        recipe{sim::baseline_recipe()},
        rng{1471},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State route_state(const bool second_energy_in_deck) {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  state.hand = {sim::Card::EarthenVessel, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Arven};
  state.prizes = {sim::Card::Fire, sim::Card::Gladion};
  if (second_energy_in_deck) {
    state.deck[1] = sim::Card::Fire;
    state.prizes[0] = sim::Card::Arven;
  }
  return state;
}

void test_k0_hidden_energy_placement_makes_same_safe_decision() {
  const auto verify = [](sim::State state, const char* label) {
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state), false);

    // Both physical hidden placements have identical public information. K0 must use
    // fixed-list and public-zone possible counts, preserve Vessel, and keep the held
    // strict-JIT Dragon until a legal inspection establishes the exact inventory:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv8-130
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Candidate bug: https://github.com/FlareZ123/pokemon-sims/issues/1471
    if (!sim::EngineTestAccess::hold_vessel(fixture.engine)) {
      throw std::runtime_error(std::string(label) +
                               ": K0 failed to preserve the uncertain route");
    }
    if (sim::EngineTestAccess::play_vessel(fixture.engine)) {
      throw std::runtime_error(std::string(label) +
                               ": K0 spent Vessel before legal inspection");
    }
    const auto& after = sim::EngineTestAccess::state(fixture.engine);
    if (std::find(after.hand.begin(), after.hand.end(), sim::Card::Dragapult) ==
        after.hand.end()) {
      throw std::runtime_error(std::string(label) +
                               ": K0 discarded the protected Dragon");
    }
  };

  verify(route_state(true), "second Energy physically in deck");
  verify(route_state(false), "second Energy physically in Prizes");
}

void test_k1_uses_exact_energy_inventory() {
  Fixture sufficient;
  sim::EngineTestAccess::set_state(sufficient.engine, route_state(true), true);
  expect(sim::EngineTestAccess::hold_vessel(sufficient.engine),
         "K1 did not preserve the proved two-Energy route");

  Fixture insufficient;
  sim::EngineTestAccess::set_state(insufficient.engine, route_state(false), true);
  expect(!sim::EngineTestAccess::hold_vessel(insufficient.engine),
         "K1 ignored the proved insufficient Energy inventory");
}

}  // namespace

int main() {
  test_k0_hidden_energy_placement_makes_same_safe_decision();
  test_k1_uses_exact_energy_inventory();
  return 0;
}

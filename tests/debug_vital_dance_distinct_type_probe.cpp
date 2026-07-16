#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool bench_oricorio_if_useful(Engine& engine) {
    return engine.bench_oricorio_if_useful();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

int main() {
  sim::Scenario scenario{"debug-vital-dance-distinct-type", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Oricorio};
  state.deck = {sim::Card::Grass, sim::Card::Grass};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{784};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Vital Dance may search up to two Basic Energy only when they have different types:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // Earthen Vessel lacks that restriction and is the required control:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  if (!sim::EngineTestAccess::bench_oricorio_if_useful(engine)) {
    throw std::runtime_error("The deterministic Vital Dance route did not resolve.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  const int grass = static_cast<int>(
      std::count(after.hand.begin(), after.hand.end(), sim::Card::Grass));
  const int fire = static_cast<int>(
      std::count(after.hand.begin(), after.hand.end(), sim::Card::Fire));
  std::cout << "vital_dance_grass=" << grass << " vital_dance_fire=" << fire << '\n';

  // Validation-only probe: current main reproduces issue #784 by returning two Grass.
  // https://github.com/FlareZ123/pokemon-sims/issues/784
  if (grass != 2 || fire != 0) {
    throw std::runtime_error("Current-main issue #784 reproduction changed unexpectedly.");
  }
  return 0;
}

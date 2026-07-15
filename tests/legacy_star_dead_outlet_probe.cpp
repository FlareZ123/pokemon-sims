#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

struct ProbeResult {
  bool used;
  bool payload_ready;
  std::size_t deck_size;
  std::size_t discard_size;
};

ProbeResult probe(const sim::Card outlet, const std::uint64_t seed) {
  const sim::Scenario scenario{"legacy-star-dead-outlet-probe",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {outlet, sim::Card::Dipplin};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Quick Ball searches only Basic Pokémon, while Mega Dragonite ex is Stage 2:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Earthen Vessel searches only Basic Energy, and this deck has none:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // Legacy Star can discard the sole deck card and establish the payload axis:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 must use the inspected deck when selecting the shortest live route:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  const bool used = sim::EngineTestAccess::use_legacy_star(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  return ProbeResult{used, sim::EngineTestAccess::payload_ready(engine),
                     after.deck.size(), after.discard.size()};
}

void require_current_bug(const ProbeResult& result, const char* label) {
  std::cout << label << " used=" << result.used
            << " payload_ready=" << result.payload_ready
            << " deck_size=" << result.deck_size
            << " discard_size=" << result.discard_size << '\n';
  if (result.used || result.payload_ready || result.deck_size != 1U ||
      result.discard_size != 0U) {
    throw std::runtime_error("Current-main reproduction changed; re-evaluate issue #567.");
  }
}

}  // namespace

int main() {
  try {
    require_current_bug(probe(sim::Card::QuickBall, 5671), "quick_ball");
    require_current_bug(probe(sim::Card::EarthenVessel, 5672), "earthen_vessel");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

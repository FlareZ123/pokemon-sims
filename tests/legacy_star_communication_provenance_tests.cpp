#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
};
}  // namespace sim

namespace {
struct Fixture {
  sim::Scenario scenario{"issue-936-legacy-star-communication-provenance",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{93601};
  sim::Engine engine{scenario, recipe, rng};
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void recovery_preflight_preserves_the_live_payload_and_takes_energy() {
  Fixture fixture;
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.deck = {
      sim::Card::Oricorio,
      sim::Card::PathToPeak,
      sim::Card::Lusamine,
      sim::Card::Guzma,
      sim::Card::Channeler,
      sim::Card::Grass,
      sim::Card::MegaDragonite,
      sim::Card::PokemonCommunication,
  };

  // Legacy Star discards Pokémon Communication, Mega Dragonite ex, and Grass
  // Energy. Projecting Mega Dragonite ex into hand must remove its copied same-turn
  // provenance, causing the selector to keep that payload in discard and recover the
  // Grass Energy that completes GGF:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/936
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star recovery route was rejected");
  }
  if (!contains(state.hand, sim::Card::Grass)) {
    throw std::runtime_error("Legacy Star did not recover the missing Grass Energy");
  }
  if (contains(state.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Legacy Star incorrectly recovered the sole current-turn payload");
  }
  if (contains(state.hand, sim::Card::PokemonCommunication)) {
    throw std::runtime_error("Legacy Star incorrectly selected the stale Communication exchange");
  }
  if (!contains(state.discard, sim::Card::MegaDragonite) ||
      !contains(state.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Legacy Star lost the strict-JIT payload or its provenance");
  }
  if (!contains(state.discard, sim::Card::PokemonCommunication)) {
    throw std::runtime_error("Legacy Star unexpectedly removed Pokémon Communication");
  }
}
}  // namespace

int main() {
  recovery_preflight_preserves_the_live_payload_and_takes_energy();
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static void play_basics_from_hand(Engine& engine) { engine.play_basics_from_hand(); }
  static void attach_manual(Engine& engine) { engine.attach_manual(); }
  static bool retreat_to_benched_vstar_with_latias(Engine& engine) {
    return engine.retreat_to_benched_vstar_with_latias();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

std::vector<sim::Card> seven_non_connector_cards() {
  return {sim::Card::ChaoticSwell, sim::Card::PathToPeak, sim::Card::MawileGX,
          sim::Card::Dipplin, sim::Card::Guzma, sim::Card::Channeler,
          sim::Card::FieldBlower};
}

void test_legacy_star_recovers_latias_for_active_vstar() {
  const sim::Scenario scenario{"legacy-star-latias", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{122};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None});
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Crispin, sim::Card::Arven,
                sim::Card::MawileGX, sim::Card::LatiasEx, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Legacy Star may discard the top 7 cards, then recover up to 2 cards from discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("Legacy Star should be a live random payload route.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::LatiasEx)) {
    throw std::runtime_error("Legacy Star should recover Latias ex for the missing Active-position axis.");
  }

  // Skyliner gives Basic Pokémon in play no Retreat Cost, allowing Regidrago V to
  // retreat so the powered Benched VSTAR becomes Active: https://api.pokemontcg.io/v2/cards/sv8-76
  sim::EngineTestAccess::play_basics_from_hand(engine);
  if (!sim::EngineTestAccess::retreat_to_benched_vstar_with_latias(engine)) {
    throw std::runtime_error("Recovered Latias ex should enable the free retreat into Regidrago VSTAR.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoVstar ||
      !contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The Latias recovery line must preserve an Active GGF VSTAR and the same-turn payload.");
  }
}

void test_legacy_star_recovers_energy_after_payload_is_ready() {
  const sim::Scenario scenario{"legacy-star-known-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2881};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None};
  state.discard = {sim::Card::MegaDragonite, sim::Card::Grass};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = seven_non_connector_cards();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Legacy Star can return any two cards from discard even when the payload axis is
  // already satisfied: https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("Legacy Star should recover a known Energy that completes GGF.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Grass)) {
    throw std::runtime_error("Legacy Star should put the needed Grass Energy into hand.");
  }

  // The turn procedure permits one manual Energy attachment:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::attach_manual(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->grass != 2 || after.active->fire != 1 ||
      !after.vstar_power_used || !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The recovered Energy must complete GGF without invalidating the strict-JIT payload.");
  }
}

void test_legacy_star_recovers_latias_after_payload_is_ready() {
  const sim::Scenario scenario{"legacy-star-known-latias", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2882};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None});
  state.discard = {sim::Card::MegaDragonite, sim::Card::LatiasEx};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = seven_non_connector_cards();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Legacy Star may recover Latias ex from an existing discard even though no new
  // payload is needed: https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::use_legacy_star(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::LatiasEx)) {
    throw std::runtime_error("Legacy Star should recover the known Latias active-axis connector.");
  }

  // Skyliner makes the Basic Active's Retreat Cost zero:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  sim::EngineTestAccess::play_basics_from_hand(engine);
  if (!sim::EngineTestAccess::retreat_to_benched_vstar_with_latias(engine)) {
    throw std::runtime_error("The recovered Latias should promote the powered Benched VSTAR.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoVstar ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The known Latias recovery must complete the Active axis and preserve payload timing.");
  }
}

}  // namespace

int main() {
  try {
    test_legacy_star_recovers_latias_for_active_vstar();
    test_legacy_star_recovers_energy_after_payload_is_ready();
    test_legacy_star_recovers_latias_after_payload_is_ready();
    std::cout << "legacy star Latias recovery tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

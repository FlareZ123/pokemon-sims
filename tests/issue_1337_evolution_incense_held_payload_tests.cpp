#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_evolution_incense(Engine& engine) {
    return engine.play_evolution_incense(true);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State payload_state(const bool include_payload) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure,
                sim::Card::Grass};
  if (include_payload) state.hand.push_back(sim::Card::MegaDragonite);
  state.deck = {sim::Card::Dragapult, sim::Card::TapuLeleGX,
                sim::Card::RegidragoV};
  return state;
}

void test_held_payload_preserves_evolution_incense() {
  const sim::Scenario scenario{"issue-1337-held-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1337};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, payload_state(true));

  // Mysterious Treasure can discard the public Mega Dragonite ex immediately.
  // Evolution Incense would only fetch a second payload and consume a one-shot Item:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1337
  expect(!sim::EngineTestAccess::play_evolution_incense(engine),
         "Evolution Incense must hold when a payload and direct outlet are public.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::EvolutionIncense),
         "The redundant Evolution Incense must remain in hand.");
  expect(contains(result.hand, sim::Card::MegaDragonite),
         "The held payload must remain available for Mysterious Treasure.");
  expect(contains(result.deck, sim::Card::Dragapult),
         "The redundant second payload must remain in deck.");
}

void test_no_held_payload_preserves_incense_treasure_bridge() {
  const sim::Scenario scenario{"issue-1337-no-held-payload",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1338};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, payload_state(false));

  // With no payload in hand, Evolution Incense may fetch Dragapult ex for the held
  // Mysterious Treasure continuation:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://github.com/FlareZ123/pokemon-sims/issues/1337
  expect(sim::EngineTestAccess::play_evolution_incense(engine),
         "The no-held-payload Incense-Treasure bridge must remain live.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::Dragapult),
         "Evolution Incense must fetch Dragapult ex in the positive control.");
}

void test_missing_vstar_axis_overrides_held_payload_guard() {
  const sim::Scenario scenario{"issue-1337-vstar-axis",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1339};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::TapuLeleGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Evolution Incense must still solve the separate Regidrago VSTAR card axis even
  // when a Dragon payload is already held:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1337
  expect(sim::EngineTestAccess::play_evolution_incense(engine),
         "Evolution Incense must remain live for the missing VSTAR axis.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::RegidragoVstar),
         "Evolution Incense must search Regidrago VSTAR in the axis control.");
}

}  // namespace

int main() {
  try {
    test_held_payload_preserves_evolution_incense();
    test_no_held_payload_preserves_incense_treasure_bridge();
    test_missing_vstar_axis_overrides_held_payload_guard();
    std::cout << "Issue 1337 Evolution Incense held-payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

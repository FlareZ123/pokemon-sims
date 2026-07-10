#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool use_celestial_roar(Engine& engine) { return engine.use_celestial_roar(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_strict_jit_holds_when_known_deck_has_no_needed_energy() {
  const sim::Scenario scenario{"celestial-roar-dead-strict", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3101};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.deck = {sim::Card::Grass, sim::Card::MegaDragonite, sim::Card::Arven};
  sim::EngineTestAccess::set_deck_seen(engine);

  // Celestial Roar attaches only Energy among the discarded top 3. K1 proves that
  // no Fire Energy remains, and strict JIT cannot carry this turn's payload discard
  // into a later ready turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  assert(!sim::EngineTestAccess::use_celestial_roar(engine));
  assert(!state.turn_ended);
  assert(state.deck.size() == 3U);
  assert(!contains(state.discard, sim::Card::MegaDragonite));
  assert(state.active->fire == 0);
}

void test_no_control_keeps_live_payload_banking_attack() {
  const sim::Scenario scenario{"celestial-roar-payload-bank", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3102};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.deck = {sim::Card::Grass, sim::Card::MegaDragonite, sim::Card::Arven};
  sim::EngineTestAccess::set_deck_seen(engine);

  // No-discard-control may bank a Dragon payload before the eventual Apex Dragon
  // turn, so the known payload in Celestial Roar's possible top-three set remains a
  // live modeled route even though no needed Fire Energy remains:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(sim::EngineTestAccess::use_celestial_roar(engine));
  assert(state.turn_ended);
  assert(contains(state.discard, sim::Card::MegaDragonite));
}

void test_strict_jit_attacks_when_needed_fire_may_remain() {
  const sim::Scenario scenario{"celestial-roar-live-fire", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3103};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.deck = {sim::Card::Arven, sim::Card::MegaDragonite, sim::Card::Fire};
  sim::EngineTestAccess::set_deck_seen(engine);

  // A known remaining Fire Energy can still be attached by Celestial Roar:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  assert(sim::EngineTestAccess::use_celestial_roar(engine));
  assert(state.turn_ended);
  assert(state.active->fire == 1);
}

}  // namespace

int main() {
  test_strict_jit_holds_when_known_deck_has_no_needed_energy();
  test_no_control_keeps_live_payload_banking_attack();
  test_strict_jit_attacks_when_needed_fire_may_remain();
  return 0;
}

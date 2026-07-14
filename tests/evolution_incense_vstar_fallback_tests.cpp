#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_evolution_incense(Engine& engine) {
    return engine.play_evolution_incense(true);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"evolution-incense-vstar-fallback",
                         sim::DciProfile::StrictJit, sim::LockMode::None,
                         false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{532};
  sim::Engine engine{scenario, recipe, rng};
};

void test_k0_search_takes_regidrago_vstar_as_legal_fallback() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // K0 permits the plausible Evolution-payload search while one card remains for
  // the later Mysterious Treasure search. Once inspection proves no modeled payload
  // remains, Regidrago VSTAR is still a legal Evolution Pokémon fallback:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  assert(sim::EngineTestAccess::play_evolution_incense(fixture.engine));
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  assert(contains(after.hand, sim::Card::RegidragoVstar));
  assert(!contains(after.deck, sim::Card::RegidragoVstar));
  assert(contains(after.deck, sim::Card::Grass));
  assert(contains(after.discard, sim::Card::EvolutionIncense));
}

void test_k1_known_dead_payload_route_still_holds_incense() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // At K1 the deck is known to contain no modeled Evolution Dragon payload. The
  // legal Regidrago VSTAR fallback cannot justify starting that known-dead payload
  // route, so the pre-search hold policy remains unchanged:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  assert(!sim::EngineTestAccess::play_evolution_incense(fixture.engine));
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  assert(contains(after.hand, sim::Card::EvolutionIncense));
  assert(contains(after.hand, sim::Card::MysteriousTreasure));
  assert(contains(after.deck, sim::Card::RegidragoVstar));
  assert(after.discard.empty());
}

}  // namespace

int main() {
  test_k0_search_takes_regidrago_vstar_as_legal_fallback();
  test_k1_known_dead_payload_route_still_holds_incense();
  return 0;
}

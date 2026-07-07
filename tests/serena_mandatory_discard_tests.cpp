#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_serena(Engine& engine) { return engine.play_serena(); }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

void test_serena_requires_a_discard() {
  using namespace sim;
  const Scenario scenario{"serena-mandatory-discard", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(164);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena};
  state.deck = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::QuickBall};

  // Exact Serena draw text says "You must discard at least 1 card"; zero is illegal: https://api.pokemontcg.io/v2/cards/swsh12-164
  assert(!EngineTestAccess::play_serena(engine));
  assert(!state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Serena) == 1);
  assert(state.discard.empty());
}

void test_evolution_incense_is_not_a_hand_payload_discard_route() {
  using namespace sim;
  const Scenario scenario{"evolution-incense-not-discard", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(201);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::EvolutionIncense};
  state.deck = {Card::MegaDragonite};

  // Mysterious Treasure can fetch a Dragon after a hand discard, while Evolution Incense has no discard effect: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-163
  assert(!EngineTestAccess::play_mysterious_treasure(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MysteriousTreasure) == 1);
  assert(std::count(state.deck.begin(), state.deck.end(), Card::MegaDragonite) == 1);
}

void test_spent_supporter_slot_disables_serena_payload_continuation() {
  using namespace sim;
  const Scenario scenario{"spent-serena-payload-outlet", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(204);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.supporter_used = true;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::Serena};
  state.deck = {Card::MegaDragonite};

  // Serena cannot be used after the turn's Supporter play: https://api.pokemontcg.io/v2/cards/swsh12-164 https://www.pokemon.com/us/pokemon-tcg/rules
  assert(!EngineTestAccess::play_mysterious_treasure(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MysteriousTreasure) == 1);
  assert(std::count(state.deck.begin(), state.deck.end(), Card::MegaDragonite) == 1);
}

}  // namespace

int main() {
  test_serena_requires_a_discard();
  test_evolution_incense_is_not_a_hand_payload_discard_route();
  test_spent_supporter_slot_disables_serena_payload_continuation();
  return 0;
}

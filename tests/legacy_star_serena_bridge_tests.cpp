#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_evolution_incense(Engine& engine) {
    return engine.play_evolution_incense(true);
  }
  static bool play_serena(Engine& engine) { return engine.play_serena(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_legacy_star_recovers_incense_and_serena_payload_bridge() {
  using namespace sim;
  const Scenario scenario{"legacy-star-incense-serena", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(451);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  // Legacy Star discards from the back. Mega Dragonite remains as the sole deck card
  // while Evolution Incense and Serena are among the seven discarded cards.
  state.deck = {Card::MegaDragonite, Card::Guzma, Card::Channeler, Card::FieldBlower,
                Card::MawileGX, Card::Dipplin, Card::EvolutionIncense, Card::Serena};

  // Legacy Star may recover any two cards. Evolution Incense then searches the
  // Evolution Dragon, and Serena's draw mode must discard at least one hand card,
  // allowing that fetched Dragon to establish the current-turn Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(contains(state.hand, Card::EvolutionIncense));
  assert(contains(state.hand, Card::Serena));
  assert(state.deck == std::vector<Card>{Card::MegaDragonite});

  assert(EngineTestAccess::play_evolution_incense(engine));
  assert(state.deck.empty());
  assert(contains(state.hand, Card::MegaDragonite));
  assert(EngineTestAccess::play_serena(engine));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(state.supporter_used);
  assert(EngineTestAccess::payload_ready(engine));
}

void test_legacy_star_does_not_recover_serena_after_supporter_use() {
  using namespace sim;
  const Scenario scenario{"legacy-star-incense-serena-spent-supporter", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(452);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.supporter_used = true;
  state.deck = {Card::MegaDragonite, Card::Guzma, Card::Channeler, Card::FieldBlower,
                Card::MawileGX, Card::Dipplin, Card::EvolutionIncense, Card::Serena};

  // A player may use only one Supporter during a turn. With that play spent, Serena
  // cannot be the same-turn discard half of the recovered Incense bridge:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(!contains(state.hand, Card::Serena));
  assert(!contains(state.hand, Card::EvolutionIncense));
  assert(contains(state.discard, Card::Serena));
  assert(contains(state.discard, Card::EvolutionIncense));
}

}  // namespace

int main() {
  test_legacy_star_recovers_incense_and_serena_payload_bridge();
  test_legacy_star_does_not_recover_serena_after_supporter_use();
  return 0;
}

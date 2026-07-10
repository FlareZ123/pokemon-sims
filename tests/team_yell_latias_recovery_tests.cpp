#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_team_yell_restores_payable_latias_promotion_route() {
  using namespace sim;
  const Scenario scenario{"team-yell-latias", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(2921);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench.push_back(Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None});
  state.hand = {Card::TeamYellsCheer, Card::QuickBall, Card::Dipplin};
  state.deck = {Card::Grass};
  state.discard = {Card::LatiasEx, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Team Yell's Cheer can restore Latias ex from discard, Quick Ball can search that
  // Basic after discarding another card, and Skyliner gives the Basic Active no
  // Retreat Cost for the Regidrago VSTAR promotion:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);

  assert(state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(contains(state.discard, Card::TeamYellsCheer));
  assert(contains(state.discard, Card::QuickBall));
  assert(!contains(state.discard, Card::LatiasEx));
  assert(!contains(state.hand, Card::LatiasEx));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const Pokemon& pokemon) {
    return pokemon.card == Card::LatiasEx;
  }));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
}

void test_team_yell_holds_when_latias_search_cost_is_unpayable() {
  using namespace sim;
  const Scenario scenario{"team-yell-latias-unpayable", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(2922);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench.push_back(Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None});
  state.hand = {Card::TeamYellsCheer, Card::QuickBall};
  state.deck = {Card::Grass};
  state.discard = {Card::LatiasEx, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Quick Ball must discard another card. After Team Yell's Cheer leaves hand, no
  // card remains to pay that cost, so the Supporter route must be preserved:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  EngineTestAccess::run_turn(engine);

  assert(!state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::TapuLeleGX);
  assert(!state.retreat_used);
  assert(contains(state.hand, Card::TeamYellsCheer));
  assert(contains(state.hand, Card::QuickBall));
  assert(contains(state.discard, Card::LatiasEx));
}

}  // namespace

int main() {
  test_team_yell_restores_payable_latias_promotion_route();
  test_team_yell_holds_when_latias_search_cost_is_unpayable();
  return 0;
}

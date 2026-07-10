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
  // Isolate Team Yell's Cheer cost validation from Legacy Star. A player may use
  // only one VSTAR Power during a game, so a previously spent power makes Legacy
  // Star unavailable without changing the Supporter and Item route under test:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  state.vstar_power_used = true;

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

void test_team_yell_restores_vstar_and_latias_in_one_resolution() {
  using namespace sim;
  const Scenario scenario{"team-yell-vstar-latias", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(324);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench.push_back(Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None});
  state.hand = {Card::TeamYellsCheer, Card::EvolutionIncense, Card::QuickBall, Card::Dipplin};
  state.deck = {Card::Grass};
  state.discard = {Card::RegidragoVstar, Card::LatiasEx, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Team Yell's Cheer may restore both Pokémon in one "up to 3" selection.
  // Evolution Incense finds Regidrago VSTAR, Quick Ball finds Latias ex, and
  // Skyliner supplies the free retreat to the newly evolved Benched attacker:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Retreat_Your_Active_Pokemon
  EngineTestAccess::run_turn(engine);

  assert(state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(state.retreat_used);
  assert(contains(state.discard, Card::TeamYellsCheer));
  assert(contains(state.discard, Card::EvolutionIncense));
  assert(contains(state.discard, Card::QuickBall));
  assert(contains(state.discard, Card::Dipplin));
  assert(!contains(state.discard, Card::RegidragoVstar));
  assert(!contains(state.discard, Card::LatiasEx));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const Pokemon& pokemon) {
    return pokemon.card == Card::LatiasEx;
  }));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
}

void test_team_yell_does_not_reuse_incense_as_quick_ball_cost() {
  using namespace sim;
  const Scenario scenario{"team-yell-no-shared-search-cost", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(325);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench.push_back(Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None});
  state.hand = {Card::TeamYellsCheer, Card::EvolutionIncense, Card::QuickBall};
  state.deck = {Card::Grass};
  state.discard = {Card::RegidragoVstar, Card::LatiasEx, Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Evolution Incense is consumed while searching Regidrago VSTAR, so it cannot
  // later become Quick Ball's required discard cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  EngineTestAccess::run_turn(engine);

  assert(state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::TapuLeleGX);
  assert(!state.retreat_used);
  assert(contains(state.discard, Card::TeamYellsCheer));
  assert(contains(state.discard, Card::EvolutionIncense));
  assert(contains(state.hand, Card::QuickBall));
  assert(contains(state.discard, Card::LatiasEx));
  assert(std::any_of(state.bench.begin(), state.bench.end(), [](const Pokemon& pokemon) {
    return pokemon.card == Card::RegidragoVstar && pokemon.grass == 2 && pokemon.fire == 1;
  }));
}

}  // namespace

int main() {
  test_team_yell_restores_payable_latias_promotion_route();
  test_team_yell_holds_when_latias_search_cost_is_unpayable();
  test_team_yell_restores_vstar_and_latias_in_one_resolution();
  test_team_yell_does_not_reuse_incense_as_quick_ball_cost();
  return 0;
}

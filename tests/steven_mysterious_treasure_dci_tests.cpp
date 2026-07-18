#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const bool going_first = false,
          const sim::LockMode lock = sim::LockMode::None)
      : scenario{"issue-775", sim::DciProfile::StrictJit, lock, going_first, 4},
        recipe(sim::baseline_recipe()), rng(775), engine(scenario, recipe, rng) {}
};

sim::State live_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::RoseannesBackup,
                sim::Card::MysteriousTreasure, sim::Card::Serena,
                sim::Card::Fire, sim::Card::MawileGX, sim::Card::Grass};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

void test_exact_route_discards_fire_and_finds_regidrago() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, live_route_state());

  // Mysterious Treasure may discard Fire Energy and search Regidrago V before
  // Steven's Resolve ends the turn. Grass remains for the manual attachment:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/775
  expect(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
         "The exact public route should make Mysterious Treasure playable.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.discard, sim::Card::Fire),
         "The route-conditioned cost should discard Fire Energy.");
  expect(contains(after.hand, sim::Card::Grass),
         "Grass Energy must remain for the turn-one manual attachment.");
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Mysterious Treasure should search Regidrago V.");
}

void test_exact_turn_establishes_regidrago_before_steven() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, live_route_state());
  sim::EngineTestAccess::run_turn(fixture.engine);

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  const auto regidrago = std::find_if(after.bench.begin(), after.bench.end(),
      [](const sim::Pokemon& pokemon) { return pokemon.card == sim::Card::RegidragoV; });
  expect(regidrago != after.bench.end(),
         "Regidrago V should enter play before Steven ends the turn.");
  expect(regidrago->grass == 1,
         "The preserved Grass Energy should be attached before Steven ends the turn.");
  expect(contains(after.discard, sim::Card::Fire),
         "The full action sequence should spend Fire on Mysterious Treasure.");
  expect(contains(after.discard, sim::Card::StevensResolve),
         "Steven should resolve after the Basic and attachment actions.");
}

void test_public_route_controls_preserve_strict_dci() {
  {
    Fixture fixture(true);
    sim::EngineTestAccess::set_state(fixture.engine, live_route_state());
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "Going first must retain the first-turn Supporter restriction route guard.");
  }
  {
    Fixture fixture(false, sim::LockMode::FullItem);
    sim::EngineTestAccess::set_state(fixture.engine, live_route_state());
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "Full Item lock must keep Mysterious Treasure unavailable.");
  }
  {
    Fixture fixture;
    sim::State state = live_route_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Grass),
                     state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "The route must preserve Grass for the turn-one manual attachment.");
  }
  {
    Fixture fixture;
    sim::State state = live_route_state();
    state.active = sim::Pokemon{sim::Card::MawileGX, 0};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "The route requires Latias ex to preserve the turn-two promotion axis.");
  }
}

void test_known_unavailable_targets_preserve_strict_dci() {
  const std::pair<sim::Card, int> unavailable_cards[] = {
      {sim::Card::RegidragoV, 4},
      {sim::Card::Crispin, 2},
      {sim::Card::BrilliantBlender, 1},
  };
  for (const auto& unavailable : unavailable_cards) {
    Fixture fixture;
    sim::State state = live_route_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), unavailable.first),
                     state.deck.end());
    for (int copy = 0; copy < unavailable.second; ++copy) {
      state.discard.push_back(unavailable.first);
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
           "A publicly exhausted route component must retain strict-DCI protection.");
  }
}
}  // namespace

int main() {
  test_exact_route_discards_fire_and_finds_regidrago();
  test_exact_turn_establishes_regidrago_before_steven();
  test_public_route_controls_preserve_strict_dci();
  test_known_unavailable_targets_preserve_strict_dci();
  return 0;
}

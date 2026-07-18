#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static bool bench_tapu_if_useful(Engine& engine) { return engine.bench_tapu_if_useful(); }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario() {
  return sim::Scenario{"steven-missing-regi", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
}

sim::State missing_regi_state(const std::vector<sim::Card>& hand) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = hand;
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void expect_steven_setup_targets(const sim::State& state) {
  expect(contains(state.hand, sim::Card::RegidragoV),
         "Steven's Resolve should establish the missing Regidrago V axis.");
  expect(contains(state.hand, sim::Card::Crispin),
         "Steven's Resolve should retain the turn-two Energy connector.");
  expect(contains(state.hand, sim::Card::BrilliantBlender),
         "Steven's Resolve should retain the payload outlet.");
  expect(state.turn_ended, "Steven's Resolve must end the turn after its legal three-card search.");
}

void test_direct_steven_fetches_missing_regidrago() {
  // Steven's Resolve searches for up to 3 cards and ends the turn: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago V is a Basic Pokémon that may be selected as one of those cards: https://api.pokemontcg.io/v2/cards/swsh12-135
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{103};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, missing_regi_state({sim::Card::StevensResolve}));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven's Resolve should be playable when it can establish the missing Regidrago V axis.");
  expect_steven_setup_targets(sim::EngineTestAccess::state(engine));
}

void test_wonder_tag_fetches_steven_for_missing_regidrago() {
  // Wonder Tag can search the deck for a Supporter when Tapu Lele-GX is Benched from hand: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{104};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state({sim::Card::TapuLeleGX});
  state.deck.push_back(sim::Card::StevensResolve);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::bench_tapu_if_useful(engine),
         "Wonder Tag should be a live connector when Steven's Resolve establishes Regidrago V.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::StevensResolve),
         "Wonder Tag should fetch Steven's Resolve for the missing Regidrago V setup line.");
  expect(sim::EngineTestAccess::play_steven(engine),
         "The fetched Steven's Resolve should establish the three-card setup line.");
  expect_steven_setup_targets(sim::EngineTestAccess::state(engine));
}

sim::State setup_before_steven_state() {
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

void test_treasure_discards_fire_before_steven() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{775};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, setup_before_steven_state());

  // Mysterious Treasure may discard Fire Energy and search Regidrago V before
  // Steven's Resolve ends the turn. Grass remains for the manual attachment:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/775
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "The exact setup-before-Steven route should make Mysterious Treasure playable.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::Fire),
         "The route-conditioned cost should discard Fire Energy.");
  expect(contains(after.hand, sim::Card::Grass),
         "Grass Energy must remain for the turn-one manual attachment.");
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Mysterious Treasure should search Regidrago V.");
}

void test_full_turn_establishes_regidrago_before_steven() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{776};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, setup_before_steven_state());
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  const auto regidrago = std::find_if(after.bench.begin(), after.bench.end(),
      [](const sim::Pokemon& pokemon) { return pokemon.card == sim::Card::RegidragoV; });
  expect(regidrago != after.bench.end(),
         "Regidrago V should enter play before Steven ends the turn.");
  expect(regidrago->grass == 1,
         "The preserved Grass Energy should attach before Steven ends the turn.");
  expect(contains(after.discard, sim::Card::Fire),
         "The full action sequence should spend Fire on Mysterious Treasure.");
  expect(contains(after.discard, sim::Card::StevensResolve),
         "Steven should resolve after the Basic and attachment actions.");
}

void test_treasure_route_controls_preserve_strict_dci() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  {
    sim::Scenario going_first = scenario();
    going_first.going_first = true;
    std::mt19937_64 rng{777};
    sim::Engine engine(going_first, recipe, rng);
    sim::EngineTestAccess::set_state(engine, setup_before_steven_state());
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "Going first must retain the first-turn Supporter restriction route guard.");
  }
  {
    sim::Scenario item_lock = scenario();
    item_lock.locks = sim::LockMode::FullItem;
    std::mt19937_64 rng{778};
    sim::Engine engine(item_lock, recipe, rng);
    sim::EngineTestAccess::set_state(engine, setup_before_steven_state());
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "Full Item lock must keep Mysterious Treasure unavailable.");
  }
  {
    std::mt19937_64 rng{779};
    sim::Engine engine(scenario(), recipe, rng);
    sim::State state = setup_before_steven_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Grass),
                     state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "The route must preserve Grass for the turn-one manual attachment.");
  }
  {
    std::mt19937_64 rng{780};
    sim::Engine engine(scenario(), recipe, rng);
    sim::State state = setup_before_steven_state();
    state.active = sim::Pokemon{sim::Card::MawileGX, 0};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "The route requires Latias ex to preserve the turn-two promotion axis.");
  }
}

void test_known_unavailable_targets_preserve_strict_dci() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  int seed = 781;
  for (const std::pair<sim::Card, int> unavailable : {
           std::pair{sim::Card::RegidragoV, 4},
           std::pair{sim::Card::Crispin, 2},
           std::pair{sim::Card::BrilliantBlender, 1}}) {
    std::mt19937_64 rng{static_cast<std::mt19937_64::result_type>(seed++)};
    sim::Engine engine(scenario(), recipe, rng);
    sim::State state = setup_before_steven_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), unavailable.first),
                     state.deck.end());
    for (int copy = 0; copy < unavailable.second; ++copy) {
      state.discard.push_back(unavailable.first);
    }
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "A publicly exhausted route component must retain strict-DCI protection.");
  }
}

}  // namespace

int main() {
  try {
    test_direct_steven_fetches_missing_regidrago();
    test_wonder_tag_fetches_steven_for_missing_regidrago();
    test_treasure_discards_fire_before_steven();
    test_full_turn_establishes_regidrago_before_steven();
    test_treasure_route_controls_preserve_strict_dci();
    test_known_unavailable_targets_preserve_strict_dci();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

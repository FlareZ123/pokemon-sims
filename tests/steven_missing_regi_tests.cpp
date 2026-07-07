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

}  // namespace

int main() {
  try {
    test_direct_steven_fetches_missing_regidrago();
    test_wonder_tag_fetches_steven_for_missing_regidrago();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

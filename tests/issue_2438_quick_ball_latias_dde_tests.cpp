#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route(const Engine& engine) {
    return engine.issue_2164_quick_ball_latias_finish_available();
  }
};
}

namespace {
void expect(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::Pokemon regidrago(int grass, int fire, int dde) {
  sim::Pokemon result{sim::Card::RegidragoV, 3, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

void run_case(int grass, int fire, int dde, sim::Card held_basic, bool expected) {
  sim::Scenario scenario{"issue-2438", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe = sim::double_dragon_modeling_recipe();
  std::mt19937_64 rng(2438);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {regidrago(grass, fire, dde)};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, held_basic};
  state.deck = {sim::Card::LatiasEx};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball and Latias ex establish the promotion connector:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // DDE plus either Basic pays Apex Dragon:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2438
  expect(sim::EngineTestAccess::route(engine) == expected,
         "Quick Ball-Latias route result mismatch");
}
}

int main() {
  try {
    run_case(0, 0, 1, sim::Card::Grass, true);
    run_case(0, 0, 1, sim::Card::Fire, true);
    run_case(2, 0, 0, sim::Card::Fire, true);
    run_case(1, 0, 0, sim::Card::Grass, false);
    std::cout << "Issue 2438 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

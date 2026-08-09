\
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_turn(Engine& engine, const int turn) { engine.state_.turn = turn; }
  static bool team_yell_powered(const Engine& engine, const Pokemon& pokemon) {
    return engine.team_yell_prior_turn_regi_v_pays_apex(pokemon);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon regi_v(const int entered_turn, const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoV, entered_turn, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2420", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2420};
  sim::Engine engine{scenario, recipe, rng};
  Fixture() { sim::EngineTestAccess::set_turn(engine, 3); }
};

void run_tests() {
  Fixture fixture;
  // DDE + either Basic pays Apex Dragon. https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex Dragon cost: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/2420
  expect(sim::EngineTestAccess::team_yell_powered(fixture.engine, regi_v(2, 1, 0, 1)),
         "missed DDE plus Grass");
  expect(sim::EngineTestAccess::team_yell_powered(fixture.engine, regi_v(2, 0, 1, 1)),
         "missed DDE plus Fire");
  expect(sim::EngineTestAccess::team_yell_powered(fixture.engine, regi_v(2, 2, 1, 0)),
         "regressed Basic GGF");
  expect(!sim::EngineTestAccess::team_yell_powered(fixture.engine, regi_v(2, 0, 0, 1)),
         "DDE-only incorrectly treated ready");
  expect(!sim::EngineTestAccess::team_yell_powered(fixture.engine, regi_v(3, 1, 0, 1)),
         "same-turn Regidrago incorrectly evolvable");
}
}  // namespace

int main() {
  try {
    run_tests();
    std::cout << "Issue 2420 Team Yell DDE readiness tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

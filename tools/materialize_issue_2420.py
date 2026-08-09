from pathlib import Path
import textwrap

source = Path("src/trace_engine_v2/part_team_yell_vstar_override.inc")
text = source.read_text(encoding="utf-8")
marker = "  bool play_team_yell_vstar_recovery() {\n"
helper = """  bool team_yell_prior_turn_regi_v_pays_apex(const Pokemon& pokemon) const {
    // Double Dragon Energy provides two Energy of every type while attached to a Dragon Pokemon.
    // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Apex Dragon costs Grass Grass Fire: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DDE semantic-readiness contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2420
    return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn &&
           pays_apex_energy_cost(pokemon);
  }

"""
if helper not in text:
    if marker not in text:
        raise SystemExit("play_team_yell_vstar_recovery marker missing")
    text = text.replace(marker, helper + marker, 1)

old_bench = """    const bool powered_benched_regi_can_evolve = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn &&
                 pokemon.grass >= 2 && pokemon.fire >= 1;
        });"""
new_bench = """    const bool powered_benched_regi_can_evolve = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return team_yell_prior_turn_regi_v_pays_apex(pokemon); // https://github.com/FlareZ123/pokemon-sims/issues/2420
        });"""
if old_bench in text:
    text = text.replace(old_bench, new_bench, 1)
elif new_bench not in text:
    raise SystemExit("powered benched predicate changed unexpectedly")

old_active = """    const bool active_regi_can_be_ready_without_retreat = state_.active &&
        state_.active->card == Card::RegidragoV && state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire >= 1;"""
new_active = """    const bool active_regi_can_be_ready_without_retreat = state_.active &&
        team_yell_prior_turn_regi_v_pays_apex(*state_.active); // https://github.com/FlareZ123/pokemon-sims/issues/2420"""
if old_active in text:
    text = text.replace(old_active, new_active, 1)
elif new_active not in text:
    raise SystemExit("active predicate changed unexpectedly")
source.write_text(text, encoding="utf-8")

test = Path("tests/issue_2420_team_yell_dde_readiness_tests.cpp")
test.write_text(textwrap.dedent(r'''\
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
'''), encoding="utf-8")

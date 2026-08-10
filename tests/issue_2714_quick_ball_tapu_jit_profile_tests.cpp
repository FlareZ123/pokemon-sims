#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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

  static bool quick_ball_tapu_steven_route(const Engine& engine) {
    return engine.issue_1797_quick_ball_tapu_steven_route_available();
  }

  static std::optional<Card> quick_ball_cost(const Engine& engine) {
    return engine.issue_1797_quick_ball_cost();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario route_scenario(const sim::DciProfile dci) {
  return sim::Scenario{"issue-2714-jit-profile-gate", dci,
                       sim::LockMode::None, true, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::TateLiza,
      sim::Card::Lusamine,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::StevensResolve,
      sim::Card::Crispin,
      sim::Card::EarthenVessel,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
  };
  state.discard = {sim::Card::HisuianHeavyBall};
  state.prizes = {
      sim::Card::ProfessorTuro,
      sim::Card::Dragapult,
      sim::Card::MysteriousTreasure,
      sim::Card::PathToPeak,
      sim::Card::Guzma,
      sim::Card::Oricorio,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2714};
  sim::Engine engine;

  explicit Fixture(const sim::DciProfile dci)
      : scenario_value(route_scenario(dci)),
        engine(scenario_value, recipe, rng) {
    sim::EngineTestAccess::set_state(engine, route_state());
  }
};

void matchup_flex_admits_same_turn_jit_route() {
  Fixture fixture{sim::DciProfile::MatchupFlexJit};

  // Strict-JIT and matchup-flex JIT both require the permitted Dragon payload on
  // the same ready turn, so this fully proven K1 connector has identical timing:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Quick Ball / Tapu Lele-GX / Steven / Crispin / Vessel:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2714
  expect(sim::EngineTestAccess::quick_ball_tapu_steven_route(fixture.engine),
         "Matchup-flex JIT rejected the complete Quick Ball-Tapu-Steven route");
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::TateLiza,
         "Matchup-flex JIT did not preserve the route-replaced Quick Ball cost");
}

void strict_jit_remains_admitted() {
  Fixture fixture{sim::DciProfile::StrictJit};
  expect(sim::EngineTestAccess::quick_ball_tapu_steven_route(fixture.engine),
         "Strict-JIT regression lost the original #1797 route");
}

void no_discard_control_remains_excluded() {
  Fixture fixture{sim::DciProfile::NoDiscardControl};

  // No-discard-control is a separate policy profile and does not share the JIT
  // payload permission used by this specialized route:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/2714
  expect(!sim::EngineTestAccess::quick_ball_tapu_steven_route(fixture.engine),
         "No-discard-control incorrectly entered the specialized JIT route");
}

}  // namespace

int main() {
  try {
    matchup_flex_admits_same_turn_jit_route();
    strict_jit_remains_admitted();
    no_discard_control_remains_excluded();
  } catch (const std::exception&) {
    return 1;
  }
  return 0;
}

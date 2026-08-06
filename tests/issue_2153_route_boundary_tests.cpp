#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
    return engine.issue_2153_steven_latias_blender_route_available();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario() {
  return sim::Scenario{"issue-2153-route-boundaries",
                       sim::DciProfile::StrictJit,
                       sim::LockMode::None,
                       false,
                       5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::DialgaGX,
      sim::Card::Dragapult,
      sim::Card::StevensResolve,
      sim::Card::EarthenVessel,
      sim::Card::Crispin,
      sim::Card::ForestSealStone,
  };
  state.deck = {
      sim::Card::RegidragoV,
      sim::Card::LatiasEx,
      sim::Card::BrilliantBlender,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::MegaDragonite,
  };
  state.prizes = {
      sim::Card::GoodraVstar,
      sim::Card::Channeler,
      sim::Card::TeamYellsCheer,
      sim::Card::QuickBall,
      sim::Card::Powerglass,
      sim::Card::FieldBlower,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value{scenario()};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2153};
  sim::Engine engine{scenario_value, recipe, rng};
};

void missing_vessel_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::EarthenVessel),
                   state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route invented Earthen Vessel");

  // Earthen Vessel is the visible cost-aware two-Energy connector:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void missing_vessel_cost_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::Crispin),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route ignored its deterministic Vessel discard cost");

  // Star Alchemy obtains the redundant Crispin that pays Vessel's discard cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void no_payload_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::MegaDragonite),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The route used Blender without a surviving permitted payload");

  // Brilliant Blender must have a legal Dragon payload for strict-JIT readiness:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void incomplete_evolution_timing_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                                     sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The seed-408 package overrode an already established Basic line");

  // The issue-2153 package is only for the missing-Basic prior-turn evolution line:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

void superior_direct_active_route_rejects_route() {
  Fixture fixture;
  sim::State state = route_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0,
                              sim::Tool::ForestSealStone};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route(fixture.engine),
         "The Latias promotion package overrode an Active Regidrago route");

  // Latias is discrete promotion value only while a Basic Active blocks the attacker:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/2153
}

}  // namespace

int main() {
  try {
    missing_vessel_rejects_route();
    missing_vessel_cost_rejects_route();
    no_payload_rejects_route();
    incomplete_evolution_timing_rejects_route();
    superior_direct_active_route_rejects_route();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

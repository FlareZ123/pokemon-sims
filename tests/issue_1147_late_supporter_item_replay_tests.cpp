#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool replay_available(const Engine& engine) {
    return engine.late_supporter_quick_ball_latias_completion_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::QuickBall, sim::Card::FieldBlower};
  state.deck = {sim::Card::LatiasEx};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_exact_route_and_controls() {
  const sim::Scenario scenario{"issue-1147", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{114700};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // After late Arven searches Quick Ball, Field Blower is a legal cost, Latias ex
  // supplies Skyliner, and the GGF plus payload axes are already complete:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1147
  expect(sim::EngineTestAccess::replay_available(engine),
         "The exact late-Supporter Quick Ball replay must be available.");

  const auto blocked = [&](sim::State state, const sim::Scenario& blocked_scenario,
                           const std::uint64_t seed, const char* message) {
    std::mt19937_64 local_rng{seed};
    sim::Engine blocked_engine = make_engine(blocked_scenario, local_rng,
                                             std::move(state));
    expect(!sim::EngineTestAccess::replay_available(blocked_engine), message);
  };

  sim::State no_cost = route_state();
  no_cost.hand.erase(std::find(no_cost.hand.begin(), no_cost.hand.end(),
                               sim::Card::FieldBlower));
  blocked(no_cost, scenario, 114701,
          "The replay must require a legal Quick Ball cost.");

  sim::State no_latias = route_state();
  no_latias.deck.clear();
  blocked(no_latias, scenario, 114702,
          "The replay must require searchable Latias ex.");

  sim::State short_energy = route_state();
  short_energy.bench.front().grass = 1;
  blocked(short_energy, scenario, 114703,
          "The replay must preserve the Item while GGF is incomplete.");

  sim::State no_payload = route_state();
  no_payload.discard.clear();
  no_payload.discarded_this_turn.clear();
  blocked(no_payload, scenario, 114704,
          "The replay must require the payload axis or a live Blender continuation.");

  sim::State full_bench = route_state();
  while (full_bench.bench.size() < 5) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  }
  blocked(full_bench, scenario, 114705,
          "The replay must require an open Bench slot.");

  const sim::Scenario item_lock{"issue-1147-item-lock",
                                sim::DciProfile::NoDiscardControl,
                                sim::LockMode::FullItem, false, 5};
  blocked(route_state(), item_lock, 114706,
          "Item lock must suppress the replay.");

  const sim::Scenario ability_lock{
      "issue-1147-ability-lock", sim::DciProfile::NoDiscardControl,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  blocked(route_state(), ability_lock, 114707,
          "Rule Box Ability lock must suppress Skyliner.");
}

void test_seed_344_reaches_turn_two() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-second");
  if (!scenario) throw std::runtime_error("Missing no-discard-control/go-second scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{344};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Seed 344 must replay the newly searched Item on T2 and complete the Latias route:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/1147
  expect(outcome.first_ready_turn == 2, "Seed 344 must become ready on turn two.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-ARVEN-01"),
         "Seed 344 must use Arven on turn two.");
  expect(trace_contains(trace, "T2 | DISCARD | rules: R-QB-01 | Field Blower"),
         "Seed 344 must use Field Blower as Quick Ball's legal cost.");
  expect(trace_contains(trace, "T2 | PLAY ITEM | rules: R-QB-01"),
         "Seed 344 must replay Quick Ball on turn two.");
  expect(trace_contains(trace, "T2 | RETREAT | rules: R-LATIAS-01"),
         "Seed 344 must use Skyliner to promote Regidrago VSTAR.");
  expect(trace_contains(trace, "T2 | READY |"),
         "Seed 344 must record turn-two readiness.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_controls();
    test_seed_344_reaches_turn_two();
    std::cout << "Issue 1147 late-Supporter Item replay tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

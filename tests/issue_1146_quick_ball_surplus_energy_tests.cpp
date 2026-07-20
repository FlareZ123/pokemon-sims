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
  static std::optional<Card> final_surplus_cost(const Engine& engine) {
    return engine.quick_ball_final_surplus_energy_cost();
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
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
  state.turn = 3;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite};
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
  const sim::Scenario scenario{"issue-1146", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{114600};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // The fully powered VSTAR makes the lone Grass surplus. Quick Ball may spend it
  // only because Latias ex plus held Blender deterministically completes T3:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1146
  expect(sim::EngineTestAccess::final_surplus_cost(engine) == sim::Card::Grass,
         "The exact final-surplus Grass route must be available.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must use the route-conditioned surplus Energy.");

  const auto blocked = [&](sim::State state, const sim::Scenario& blocked_scenario,
                           const std::uint64_t seed, const char* message) {
    std::mt19937_64 local_rng{seed};
    sim::Engine blocked_engine = make_engine(blocked_scenario, local_rng,
                                             std::move(state));
    expect(!sim::EngineTestAccess::final_surplus_cost(blocked_engine), message);
  };

  sim::State missing_energy = route_state();
  missing_energy.bench.front().grass = 1;
  blocked(missing_energy, scenario, 114601,
          "Energy must remain protected while GGF is incomplete.");

  sim::State no_blender = route_state();
  no_blender.hand.erase(std::find(no_blender.hand.begin(), no_blender.hand.end(),
                                  sim::Card::BrilliantBlender));
  blocked(no_blender, scenario, 114602,
          "The fallback must require the current-turn Blender payload route.");

  sim::State no_latias = route_state();
  no_latias.deck.erase(std::find(no_latias.deck.begin(), no_latias.deck.end(),
                                 sim::Card::LatiasEx));
  blocked(no_latias, scenario, 114603,
          "The fallback must require searchable Latias ex.");

  sim::State full_bench = route_state();
  while (full_bench.bench.size() < 5) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  }
  blocked(full_bench, scenario, 114604,
          "The fallback must require an open Bench slot.");

  sim::State attachment_open = route_state();
  attachment_open.manual_energy_used = false;
  blocked(attachment_open, scenario, 114605,
          "The fallback must remain tied to the completed attachment state.");

  const sim::Scenario strict{"issue-1146-strict", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  blocked(route_state(), strict, 114606,
          "Strict DCI must keep the singleton Energy protected.");

  const sim::Scenario item_lock{"issue-1146-item-lock",
                                sim::DciProfile::MatchupFlexJit,
                                sim::LockMode::FullItem, false, 5};
  blocked(route_state(), item_lock, 114607,
          "Item lock must suppress the Quick Ball route.");
}

void test_lower_dci_cost_keeps_priority() {
  const sim::Scenario scenario{"issue-1146-priority",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{114608};
  sim::State state = route_state();
  state.hand.push_back(sim::Card::FieldBlower);
  sim::Engine engine = make_engine(scenario, rng, std::move(state));
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must remain playable with lower-DCI fuel.");
  expect(std::find(engine.state().discard.begin(), engine.state().discard.end(),
                   sim::Card::FieldBlower) != engine.state().discard.end(),
         "Field Blower must remain ahead of the surplus Energy fallback.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::Grass) != engine.state().hand.end(),
         "Grass must remain held when lower-DCI fuel exists.");
}

void test_seed_122_reaches_turn_three() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing matchup-flex-jit/go-second scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{122};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Seed 122 must spend the observable surplus Grass on T3, then use Latias ex and
  // Brilliant Blender to finish every remaining axis on the same turn:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1146
  expect(outcome.first_ready_turn == 3, "Seed 122 must become ready on turn three.");
  expect(trace_contains(trace, "T3 | DISCARD | rules: R-QB-01 | Grass Energy"),
         "Seed 122 must spend the final surplus Grass on Quick Ball.");
  expect(trace_contains(trace, "T3 | PLAY ITEM | rules: R-QB-01"),
         "Seed 122 must search Latias ex on turn three.");
  expect(trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01"),
         "Seed 122 must use Brilliant Blender on turn three.");
  expect(trace_contains(trace, "T3 | READY |"),
         "Seed 122 must record turn-three readiness.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_controls();
    test_lower_dci_cost_keeps_priority();
    test_seed_122_reaches_turn_three();
    std::cout << "Issue 1146 Quick Ball surplus Energy tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

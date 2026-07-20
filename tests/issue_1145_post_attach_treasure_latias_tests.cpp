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
  static bool route_available(Engine& engine) {
    return engine.post_attach_treasure_latias_route_available();
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
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::Arven};
  state.deck = {sim::Card::LatiasEx};
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
  const sim::Scenario scenario{"issue-1145", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{114500};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // Once GGF is complete, the held Dragon may pay Mysterious Treasure for Latias
  // ex, whose Skyliner Ability solves the final Basic Active position directly:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1145
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact post-attachment Treasure-to-Latias route must be live.");

  const auto blocked = [&](sim::State state, const sim::Scenario& blocked_scenario,
                           const std::uint64_t seed, const char* message) {
    std::mt19937_64 local_rng{seed};
    sim::Engine blocked_engine = make_engine(blocked_scenario, local_rng,
                                             std::move(state));
    expect(!sim::EngineTestAccess::route_available(blocked_engine), message);
  };

  sim::State no_payload = route_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(),
                                  sim::Card::MegaDragonite));
  blocked(no_payload, scenario, 114501,
          "The route must require a legal current-turn payload cost.");

  sim::State no_latias = route_state();
  no_latias.deck.clear();
  blocked(no_latias, scenario, 114502,
          "The route must require Latias ex to remain searchable.");

  sim::State full_bench = route_state();
  while (full_bench.bench.size() < 5) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  }
  blocked(full_bench, scenario, 114503,
          "The route must require an open Bench slot.");

  sim::State short_energy = route_state();
  short_energy.bench.front().grass = 1;
  blocked(short_energy, scenario, 114504,
          "The route must preserve Treasure while GGF is incomplete.");

  const sim::Scenario item_lock{"issue-1145-item-lock", sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, false, 5};
  blocked(route_state(), item_lock, 114505,
          "Item lock must suppress the Treasure route.");

  const sim::Scenario ability_lock{
      "issue-1145-ability-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  blocked(route_state(), ability_lock, 114506,
          "Rule Box Ability lock must suppress Skyliner.");
}

void test_seed_five_preserves_arven() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-second scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The direct Item route must reach T4 while preserving the Supporter:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1145
  expect(outcome.first_ready_turn == 4, "Seed 5 must remain ready on turn four.");
  expect(trace_contains(trace, "T4 | PLAY ITEM | rules: R-MT-01"),
         "Seed 5 must play Mysterious Treasure on turn four.");
  expect(trace_contains(trace, "T4 | DISCARD | rules: R-MT-01 | Mega Dragonite ex"),
         "Seed 5 must use the Dragon as Treasure's strict-JIT cost.");
  expect(!trace_contains(trace, "T4 | PLAY SUPPORTER | rules: R-ARVEN-01"),
         "Seed 5 must preserve Arven when the direct Item already completes setup.");
  expect(trace_contains(trace, "T4 | READY |"),
         "Seed 5 must record turn-four readiness.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_controls();
    test_seed_five_preserves_arven();
    std::cout << "Issue 1145 post-attachment Treasure Latias tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

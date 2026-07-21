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
  static void evolve_and_replay(Engine& engine) {
    engine.evolve_best_regi_issue_1224();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

bool bench_contains(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::FullItem) {
  return sim::Scenario{"issue-1224", sim::DciProfile::StrictJit, locks, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 2, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& chosen, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(chosen, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_exact_post_evolution_replay() {
  const sim::Scenario chosen = scenario();
  std::mt19937_64 rng{122401};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(chosen, rng, route_state(), &trace);

  // Forest Seal Stone remains attached through evolution. Once the prior-turn
  // Regidrago V becomes Regidrago VSTAR, Star Alchemy may be reconsidered and search
  // Latias ex, whose Skyliner Ability supplies the legal free promotion route:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1224
  sim::EngineTestAccess::evolve_and_replay(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(bench_contains(after, sim::Card::RegidragoVstar),
         "The prior-turn Regidrago V must evolve.");
  expect(bench_contains(after, sim::Card::LatiasEx),
         "The post-evolution Star Alchemy replay must Bench Latias ex.");
  expect(after.vstar_power_used,
         "The successful replay must consume the once-per-game VSTAR Power.");
  expect(trace_contains(trace, "STAR ALCHEMY"),
         "The replay must be visible in the trace.");
}

void test_replay_guards() {
  const auto blocked = [](sim::State state, const sim::Scenario chosen,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(chosen, rng, std::move(state));
    sim::EngineTestAccess::evolve_and_replay(engine);
    expect(!bench_contains(sim::EngineTestAccess::state(engine), sim::Card::LatiasEx),
           message);
  };

  sim::State already_used = route_state();
  already_used.vstar_power_used = true;
  blocked(already_used, scenario(), 122402,
          "A spent VSTAR Power must not replay.");

  blocked(route_state(), scenario(sim::LockMode::FullRuleBoxAbility), 122403,
          "Rule Box Ability lock must suppress Star Alchemy and Skyliner.");

  sim::State same_turn = route_state();
  same_turn.bench.front().entered_turn = 3;
  blocked(same_turn, scenario(), 122404,
          "A same-turn Regidrago V cannot evolve or unlock the replay.");

  sim::State no_stone = route_state();
  no_stone.bench.front().tool = sim::Tool::None;
  blocked(no_stone, scenario(), 122405,
          "A VSTAR without Forest Seal Stone cannot use Star Alchemy.");

  sim::State full_bench = route_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Dipplin, 1});
  blocked(full_bench, scenario(), 122406,
          "A full Bench must block the Latias completion.");
}

void test_seed_12_regression() {
  const sim::Scenario chosen{"strict-jit-full-item-lock/go-second",
                             sim::DciProfile::StrictJit,
                             sim::LockMode::FullItem, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{12};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(chosen, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Seed 12 must complete the legal post-evolution route on T3.");
  expect(trace_contains(trace, "T3 | STAR ALCHEMY"),
         "Seed 12 must replay Star Alchemy after evolution on T3.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-BURNET-01"),
         "Seed 12 must use Professor Burnet for the strict-JIT payload.");
  expect(trace_contains(trace, "T3 | READY |"),
         "Seed 12 must record T3 readiness.");
}
}  // namespace

int main() {
  try {
    test_exact_post_evolution_replay();
    test_replay_guards();
    test_seed_12_regression();
    std::cout << "Issue 1224 post-evolution FSS replay tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

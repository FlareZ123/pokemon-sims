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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_pineco(Engine& engine) {
    return engine.bench_pineco_if_useful();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  const auto it = std::find_if(trace.lines.begin(), trace.lines.end(),
                               [&text](const std::string& line) {
                                 return line.find(text) != std::string::npos;
                               });
  return it == trace.lines.end()
      ? trace.lines.size()
      : static_cast<std::size_t>(std::distance(trace.lines.begin(), it));
}

void test_seed35_benches_pineco_before_turn_ending_steven() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-35 fixture is unavailable.");

  std::mt19937_64 rng(35);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Pineco is a Basic already held before Steven's Resolve ends turn one.
  // Benching it preserves ordinary prior-turn evolution as a legal fallback.
  // The later direct Treasure route keeps Pineco unevolved because it reaches
  // the same T2 deadline while preserving Forretress ex and Bench capacity:
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Core Bench, Supporter, Stadium, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1376
  expect(outcome.first_ready_turn == 2,
         "The corrected seed-35 route lost strict-JIT T2 readiness.");
  const std::size_t pineco = trace_index(trace, "T1 | BENCH | rules: R-GAME-BENCH | Pineco");
  const std::size_t steven = trace_index(trace, "T1 | PLAY SUPPORTER | rules: R-STEVEN-01");
  expect(pineco < steven,
         "Pineco was not Benched before Steven's Resolve ended turn one.");
  // Stronger route: https://github.com/FlareZ123/pokemon-sims/issues/1420
  expect(!trace_contains(trace, "under normal prior-turn timing"),
         "The direct route unnecessarily spent Forretress ex.");
  expect(!trace_contains(trace, "T2 | PLAY STADIUM | rules: R-FOREST-VITALITY-01"),
         "The preserved prior-turn fallback still played Forest of Vitality.");
}

sim::State helper_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Pineco};
  return state;
}

void test_helper_preserves_bench_capacity_gate() {
  const sim::Scenario scenario{"issue-1376-full-bench",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(137601);
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = helper_state();
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 0},
      sim::Pokemon{sim::Card::LatiasEx, 0},
      sim::Pokemon{sim::Card::Oricorio, 0},
      sim::Pokemon{sim::Card::RegidragoV, 0},
      sim::Pokemon{sim::Card::MawileGX, 0},
  };
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The new call site delegates to the existing legal Bench helper, so a full
  // five-Pokémon Bench still blocks Pineco:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1376
  expect(!sim::EngineTestAccess::bench_pineco(engine),
         "Pineco bypassed the five-Pokémon Bench limit.");
  expect(sim::EngineTestAccess::state(engine).hand.size() == 1U,
         "A blocked Pineco Bench action changed the hand.");
}

void test_helper_preserves_energy_need_and_recipe_gates() {
  const sim::Scenario scenario{"issue-1376-controls",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};

  std::mt19937_64 complete_rng(137602);
  const sim::DeckRecipe pineco_recipe = sim::pineco_recipe();
  sim::Engine complete_engine(scenario, pineco_recipe, complete_rng);
  sim::State complete = helper_state();
  complete.active->grass = 2;
  complete.active->fire = 1;
  sim::EngineTestAccess::set_state(complete_engine, std::move(complete));
  expect(!sim::EngineTestAccess::bench_pineco(complete_engine),
         "Pineco was Benched after the Energy axis was complete.");

  std::mt19937_64 shell_rng(137603);
  const sim::DeckRecipe shell_recipe = sim::baseline_recipe();
  sim::Engine shell_engine(scenario, shell_recipe, shell_rng);
  sim::EngineTestAccess::set_state(shell_engine, helper_state());
  // Pineco and Forretress are recipe-gated and absent from the shell:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#named-deck-ownership
  // https://github.com/FlareZ123/pokemon-sims/issues/1376
  expect(!sim::EngineTestAccess::bench_pineco(shell_engine),
         "The Pineco call-site behavior leaked into the shell recipe.");
}

}  // namespace

int main() {
  try {
    test_seed35_benches_pineco_before_turn_ending_steven();
    test_helper_preserves_bench_capacity_gate();
    test_helper_preserves_energy_need_and_recipe_gates();
    std::cout << "Issue 1376 T1 Pineco-before-Steven tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

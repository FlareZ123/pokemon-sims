#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

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

void test_seed_35_replays_post_legacy_quick_ball_on_turn_two() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing matchup-flex-jit/go-second scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{35};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Legacy Star recovers Arven after the earlier Item fixed point. Arven may search
  // Quick Ball, Quick Ball may discard Chaotic Swell, Latias ex may be Benched, and
  // Skyliner may promote the already-GGF Regidrago VSTAR during the same payload turn:
  // Legacy Star / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Core Supporter, Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest complete route and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1152
  expect(outcome.first_ready_turn == 2,
         "Seed 35 must become ready on turn two after the post-Legacy replay.");
  expect(trace_contains(trace, "T2 | LEGACY STAR |"),
         "Seed 35 must use Legacy Star on turn two.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-ARVEN-01"),
         "Seed 35 must use recovered Arven on turn two.");
  expect(trace_contains(trace, "T2 | DISCARD | rules: R-QB-01 | Chaotic Swell"),
         "Seed 35 must pay Quick Ball with the ordinary low-DCI Stadium cost.");
  expect(trace_contains(trace, "T2 | PLAY ITEM | rules: R-QB-01"),
         "Seed 35 must replay Quick Ball on turn two.");
  expect(trace_contains(trace, "T2 | BENCH | rules: R-GAME-BENCH | Latias ex"),
         "Seed 35 must Bench Latias ex on turn two.");
  expect(trace_contains(trace, "T2 | RETREAT | rules: R-LATIAS-01"),
         "Seed 35 must use Skyliner on turn two.");
  expect(trace_contains(trace, "T2 | READY |"),
         "Seed 35 must record turn-two readiness.");
}
}  // namespace

int main() {
  try {
    test_seed_35_replays_post_legacy_quick_ball_on_turn_two();
    std::cout << "Issue 1152 post-Legacy Supporter Item replay tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

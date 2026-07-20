#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

bool contains_line(const sim::TraceLog& trace, const std::string& first,
                   const std::string& second) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&first, &second](const std::string& line) {
                       return line.find(first) != std::string::npos &&
                           line.find(second) != std::string::npos;
                     });
}

void test_seed_101_reserves_vstar_and_burnet() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{101};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Steven reserves the known Evolution and direct deck-to-discard Supporter on T2.
  // Burnet then establishes the strict-JIT payload on the same T3 that V evolves:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1108
  expect(outcome.first_ready_turn == 3,
         "Seed 101 must reach strict-JIT readiness on T3.");
  expect(contains(trace, "known T3 Burnet route"),
         "The T2 trace must reserve VSTAR and Professor Burnet.");
  expect(contains_line(trace, "PLAY SUPPORTER", "Professor Burnet"),
         "The T3 trace must play Professor Burnet.");
  expect(!contains(trace, "exchanged Gladion for Regidrago V"),
         "Gladion must not preempt the known T3 completion.");
}
}  // namespace

int main() {
  test_seed_101_reserves_vstar_and_burnet();
  return 0;
}

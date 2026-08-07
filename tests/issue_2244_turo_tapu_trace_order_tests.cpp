#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstddef>
#include <iostream>
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

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  for (std::size_t index = 0; index < trace.lines.size(); ++index) {
    if (trace.lines[index].find(text) != std::string::npos) return index;
  }
  return trace.lines.size();
}

void test_seed_7_records_turo_before_tapu_replay() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2244 fixture is unavailable.");

  std::mt19937_64 rng(7);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  // Professor Turo must resolve before the returned Tapu Lele-GX can be replayed
  // from hand, and Wonder Tag triggers only after that later Bench play. The
  // readable trace therefore has to preserve PLAY SUPPORTER -> BENCH -> WONDER TAG.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Core Supporter, Active replacement, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Readable-trace contract: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // Confirmed chronology bug: https://github.com/FlareZ123/pokemon-sims/issues/2244
  const std::size_t supporter = trace_index(trace, "T3 | PLAY SUPPORTER | Professor Turo returned Tapu Lele-GX");
  const std::size_t bench = trace_index(trace, "T3 | BENCH | Tapu Lele-GX from hand.");
  const std::size_t wonder_tag = trace_index(trace, "T3 | WONDER TAG | Searched and revealed Gladion.");

  expect(supporter < trace.lines.size(),
         "Seed 7 did not record the Professor Turo Supporter action.");
  expect(bench < trace.lines.size(),
         "Seed 7 did not record the replayed Tapu Lele-GX Bench action.");
  expect(wonder_tag < trace.lines.size(),
         "Seed 7 did not record the replayed Tapu Lele-GX Wonder Tag action.");
  expect(supporter < bench && bench < wonder_tag,
         "Seed 7 did not preserve PLAY SUPPORTER -> BENCH -> WONDER TAG chronology.");
}
}  // namespace

int main() {
  try {
    test_seed_7_records_turo_before_tapu_replay();
    std::cout << "Issue 2244 Professor Turo trace-order tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

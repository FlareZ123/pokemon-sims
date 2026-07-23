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

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void test_seed_211_does_not_invent_secret_box_provenance() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-rulebox-ability-lock/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1437 fixture is unavailable.");

  std::mt19937_64 rng(211);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  // Secret Box must pay three other cards before it can search a Supporter.
  // Crispin's own effect does not identify how the card entered the hand:
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // https://github.com/FlareZ123/pokemon-sims/issues/1437
  expect(trace_contains(trace, "Used held Crispin before lower-impact"),
         "The held-Crispin action lost its neutral policy trace.");
  expect(!trace_contains(trace, "Secret Box searched Crispin"),
         "The trace still invents Secret Box acquisition provenance.");
  expect(!trace_contains(trace, "T2 | PLAY ITEM | rules: R-SECRET-BOX-01"),
         "The source-bound control unexpectedly played Secret Box.");
}

}  // namespace

int main() {
  test_seed_211_does_not_invent_secret_box_provenance();
  return 0;
}

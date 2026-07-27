#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
namespace sim { struct EngineTestAccess {}; }
namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}
struct Result { sim::TrialOutcome outcome; sim::TraceLog trace; };
Result run(const std::string& variant, const std::string& scenario,
           std::uint64_t seed) {
  auto selected_scenario = sim::scenario_by_label(scenario);
  const auto* deck = sim::crobat_modeling_deck_by_id(variant);
  expect(selected_scenario && deck, "fixture");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected_scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}
void exact() {
  auto result = run("crobat2-erika-channeler",
                    "no-discard-control/go-first", 27);
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Professor Turo: https://api.pokemontcg.io/v2/cards/sv4-171
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1597
  expect(result.outcome.first_ready_turn > 0 &&
             result.outcome.first_ready_turn <= 4 &&
             !result.outcome.setup_failed,
         "seed27 missed the Steven schedule deadline");
  expect(has(result.trace, "T1 | WONDER TAG") &&
             has(result.trace, "Searched and revealed Steven's Resolve") &&
             has(result.trace, "T2 | PLAY SUPPORTER") &&
             has(result.trace, "banked Regidrago VSTAR and Crispin") &&
             has(result.trace, "READY"),
         "route absent");
}
void controls() {
  auto second = run("crobat2-erika-channeler",
                    "no-discard-control/go-second", 27);
  expect(!has(second.trace, "banked Regidrago VSTAR and Crispin"),
         "go-second used the going-first route");
  auto strict = run("crobat2-erika-channeler",
                    "strict-jit/go-first", 27);
  expect(!has(strict.trace, "banked Regidrago VSTAR and Crispin"),
         "strict used the no-control route");
}
}
int main() { exact(); controls(); }

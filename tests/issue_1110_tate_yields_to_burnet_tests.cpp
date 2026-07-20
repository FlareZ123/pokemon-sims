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

void test_seed_182_uses_burnet_on_turn_three() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{182};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The shadow classifier may evaluate deterministic held Items, evolution, and
  // attachments. It cannot use the actual hidden top-seven Legacy Star outcome to
  // suppress the known Professor Burnet Supporter route:
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Hidden-order policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1110
  expect(outcome.first_ready_turn == 3,
         "Seed 182 must reach strict-JIT readiness on T3.");
  expect(contains(trace, "T3 | PLAY SUPPORTER") &&
             contains(trace, "Professor Burnet"),
         "The T3 trace must play Professor Burnet.");
  expect(!contains(trace, "T3 | HOLD SUPPORTER"),
         "The Tate hold guard must not use a private Legacy Star sample.");
}

void test_existing_deterministic_tate_routes_remain_green() {
  for (const auto& [label, seed, expected_turn] : {
           std::tuple{"matchup-flex-jit/go-first", std::uint64_t{185}, 3},
           std::tuple{"strict-jit/go-second", std::uint64_t{13}, 2}}) {
    const auto scenario = sim::scenario_by_label(label);
    if (!scenario) throw std::runtime_error("Missing control scenario");
    sim::DeckRecipe recipe{sim::baseline_recipe()};
    std::mt19937_64 rng{seed};
    sim::TraceLog trace;
    trace.enabled = true;
    sim::Engine engine(*scenario, recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();
    expect(outcome.first_ready_turn == expected_turn,
           "A deterministic held non-Supporter Tate route regressed.");
    expect(contains(trace, "HOLD SUPPORTER"),
           "The deterministic Tate control must still hold the Supporter.");
  }
}
}  // namespace

int main() {
  test_seed_182_uses_burnet_on_turn_three();
  test_existing_deterministic_tate_routes_remain_green();
  return 0;
}

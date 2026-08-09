#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstdint>
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

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  for (const std::string& line : trace.lines) {
    if (line.find(text) != std::string::npos) return true;
  }
  return false;
}

sim::TraceLog run_trace(const char* scenario_label, const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2622 fixture is unavailable.");

  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();
  return trace;
}

void test_seed_761_selects_deterministic_t3_steven_package() {
  const sim::TraceLog trace = run_trace("strict-jit/go-second", 761);

  // K1 proves that Steven can reserve VSTAR, Latias ex, and Brilliant Blender on
  // T2. The held second Grass then completes GGF on T3, Blender establishes the
  // strict-JIT Dragon payload, and Skyliner gives Basic Oricorio no Retreat Cost.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, search, attachment, evolution, Ability, Item, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, and earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2622
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(trace,
                            "deterministic next-turn VSTAR-Latias-Blender package"),
         "Seed 761 did not choose the deterministic Steven package on T2.");
  expect(!trace_contains(trace, "T2 | PLAY SUPPORTER | R-ARVEN"),
         "Seed 761 still spent T2 on Arven instead of the faster package.");
  expect(trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "R-BLENDER-01"),
         "Seed 761 did not resolve Brilliant Blender on T3.");
  expect(trace_contains(trace, "T3 | RETREAT") &&
             trace_contains(trace, "Latias ex gives the Basic Active no Retreat Cost"),
         "Seed 761 did not use Skyliner for the T3 promotion.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 761 did not reach strict-JIT readiness on T3.");
}

void test_seed_761_package_stays_closed_under_rulebox_ability_lock() {
  const sim::TraceLog trace =
      run_trace("strict-jit-rulebox-ability-lock/go-second", 761);

  // Skyliner is a Rule Box Pokémon Ability, so this package is invalid while the
  // modeled Rule Box Ability lock is active.
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // Confirmed regression scope: https://github.com/FlareZ123/pokemon-sims/issues/2622
  expect(!trace_contains(trace,
                         "deterministic next-turn VSTAR-Latias-Blender package"),
         "Issue-2622 Steven package bypassed Rule Box Ability lock.");
}

void test_seed_761_package_stays_closed_under_combined_lock() {
  const sim::TraceLog trace =
      run_trace("strict-jit-combined-lock/go-second", 761);

  // The combined lock blocks both the Blender Item axis and Skyliner Ability axis.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official Item and Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed regression scope: https://github.com/FlareZ123/pokemon-sims/issues/2622
  expect(!trace_contains(trace,
                         "deterministic next-turn VSTAR-Latias-Blender package"),
         "Issue-2622 Steven package bypassed combined lock.");
}
}  // namespace

int main() {
  try {
    test_seed_761_selects_deterministic_t3_steven_package();
    test_seed_761_package_stays_closed_under_rulebox_ability_lock();
    test_seed_761_package_stays_closed_under_combined_lock();
    std::cout << "Issue 2622 Steven/Latias/Blender package tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

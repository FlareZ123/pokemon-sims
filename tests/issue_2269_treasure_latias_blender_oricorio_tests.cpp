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
         "The registered issue-2269 fixture is unavailable.");

  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();
  return trace;
}

void test_seed_761_uses_surplus_fire_for_oricorio_t4_finish() {
  const sim::TraceLog trace = run_trace("strict-jit/go-second", 761);

  // The selected Benched Regidrago VSTAR already has GGF. Mysterious Treasure may
  // spend the setup-surplus Fire, search Basic Psychic Latias ex, and Bench it.
  // Held Brilliant Blender then supplies the current-turn Dragon payload from the
  // K1-known deck, while Skyliner gives Basic Oricorio no Retreat Cost.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Oricorio GRI 55: https://api.pokemontcg.io/v2/cards/sm2-55
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, search, discard, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2269
  expect(trace_contains(trace, "T4 | DISCARD") &&
             trace_contains(trace,
                            "Fire Energy (Mysterious Treasure issue-2269 Latias route cost)"),
         "Seed 761 did not spend route-proven surplus Fire on T4.");
  expect(trace_contains(trace, "T4 | PLAY ITEM") &&
             trace_contains(trace, "R-BLENDER-01"),
         "Seed 761 did not resolve Brilliant Blender on T4.");
  expect(trace_contains(trace, "T4 | RETREAT") &&
             trace_contains(trace, "Latias ex gives the Basic Active no Retreat Cost"),
         "Seed 761 did not use Skyliner for the T4 promotion.");
  expect(trace_contains(trace, "T4 | READY"),
         "Seed 761 did not reach strict-JIT readiness on T4.");
}

void test_seed_761_route_stays_closed_under_rulebox_ability_lock() {
  const sim::TraceLog trace =
      run_trace("strict-jit-rulebox-ability-lock/go-second", 761);

  // Rule Box Ability lock suppresses Latias ex's Skyliner. The #2269 promotion
  // route therefore remains unavailable even when the Item and Energy are held.
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // Confirmed regression scope: https://github.com/FlareZ123/pokemon-sims/issues/2269
  expect(!trace_contains(trace, "Mysterious Treasure issue-2269 Latias route cost"),
         "Issue-2269 route bypassed Rule Box Ability lock.");
  expect(!trace_contains(trace, "T4 | READY"),
         "Issue-2269 Rule Box lock control incorrectly reached T4 readiness.");
}

void test_seed_761_route_stays_closed_under_turn_two_item_lock() {
  const sim::TraceLog trace =
      run_trace("strict-jit-turn2-item-lock/go-second", 761);

  // Scheduled Item lock forbids both Mysterious Treasure and Brilliant Blender on
  // T4, so the corrected route cannot bypass the repository lock schedule.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Official Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
  // Confirmed regression scope: https://github.com/FlareZ123/pokemon-sims/issues/2269
  expect(!trace_contains(trace, "Mysterious Treasure issue-2269 Latias route cost"),
         "Issue-2269 route bypassed Item lock.");
  expect(!trace_contains(trace, "T4 | READY"),
         "Issue-2269 Item-lock control incorrectly reached T4 readiness.");
}
}  // namespace

int main() {
  try {
    test_seed_761_uses_surplus_fire_for_oricorio_t4_finish();
    test_seed_761_route_stays_closed_under_rulebox_ability_lock();
    test_seed_761_route_stays_closed_under_turn_two_item_lock();
    std::cout << "Issue 2269 Treasure/Latias/Blender Oricorio tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

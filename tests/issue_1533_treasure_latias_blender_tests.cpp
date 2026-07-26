#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

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

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_crobat_seed_one(const std::string& scenario_label) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("Missing issue-1533 scenario");
  const sim::DeckRecipe recipe = sim::make_crobat_modeling_deck(
      "crobat1-erika", {sim::Card::ErikasInvitation}).recipe;
  std::mt19937_64 rng{1};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

void test_unlocked_seed_one_reaches_t4() {
  const SeedResult result = run_crobat_seed_one("strict-jit/go-second");

  // With GGF complete, the spare Fire no longer advances the current ready axis.
  // Mysterious Treasure may pay it, Latias ex supplies Skyliner, and held Brilliant
  // Blender independently discards a current-turn Dragon payload:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, discard, search, Bench, Ability, retreat, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
  expect(result.outcome.first_ready_turn == 4 && !result.outcome.setup_failed,
         "crobat1-erika strict-JIT seed 1 must reach readiness on T4.");
  expect(trace_contains(result.trace, "T4 | PLAY ITEM | rules: R-MT-01") &&
             trace_contains(result.trace, "Latias ex") &&
             trace_contains(result.trace, "Brilliant Blender") &&
             trace_contains(result.trace, "T4 | RETREAT |") &&
             trace_contains(result.trace, "T4 | READY |"),
         "Seed 1 did not record the complete T4 Treasure-Latias-Blender route.");
  expect(!trace_contains(result.trace, "T4 | PLAY SUPPORTER |"),
         "Seed 1 unnecessarily spent the T4 Supporter action.");
}

void test_item_lock_blocks_the_item_route() {
  const SeedResult result =
      run_crobat_seed_one("strict-jit-full-item-lock/go-second");

  // Full Item lock prevents both Mysterious Treasure and Brilliant Blender, so the
  // unlocked T4 composition must remain unavailable:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Item-lock interpretation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
  expect(!trace_contains(result.trace, "T4 | PLAY ITEM | rules: R-MT-01") &&
             !trace_contains(result.trace, "T4 | PLAY ITEM | rules: R-BLENDER-01"),
         "Full Item lock admitted the issue-1533 Item route.");
}

void test_rule_box_ability_lock_blocks_skyliner() {
  const SeedResult result =
      run_crobat_seed_one("strict-jit-rulebox-ability-lock/go-second");

  // Path-style Rule Box Ability suppression disables Latias ex's Skyliner, so the
  // Basic Active cannot use this route's free retreat on T4:
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Lock interpretation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // Core Ability and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
  expect(!trace_contains(result.trace, "T4 | RETREAT |") &&
             !trace_contains(result.trace, "T4 | READY |"),
         "Rule Box Ability lock admitted the Skyliner T4 route.");
}
}  // namespace

int main() {
  try {
    test_unlocked_seed_one_reaches_t4();
    test_item_lock_blocks_the_item_route();
    test_rule_box_ability_lock_blocks_skyliner();
    std::cout << "Issue 1533 Treasure Latias Blender tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

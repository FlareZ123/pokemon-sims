#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
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

struct SeedNineResult {
  int first_ready_turn = 0;
  int t1_wonder_tags = 0;
  bool t1_wonder_tagged_steven = false;
  bool t1_held_second_tapu_for_steven_route = false;
};

SeedNineResult run_seed_nine(const char* scenario_label) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(scenario.has_value(), "issue-2715 scenario fixture unavailable");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{9};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  SeedNineResult result;
  result.first_ready_turn = outcome.first_ready_turn;
  for (const std::string& line : trace.lines) {
    if (line.find("T1 | WONDER TAG") != std::string::npos) {
      ++result.t1_wonder_tags;
      if (line.find("Steven's Resolve") != std::string::npos) {
        result.t1_wonder_tagged_steven = true;
      }
    }
    if (line.find("T1 | HOLD TAPU LELE-GX") != std::string::npos &&
        line.find("banked Steven") != std::string::npos &&
        line.find("deterministic T3 route") != std::string::npos) {
      result.t1_held_second_tapu_for_steven_route = true;
    }
  }
  return result;
}

void test_both_jit_profiles_use_the_same_seed_nine_route() {
  // Both JIT profiles require the Dragon payload to enter discard during the
  // ready turn. In the exact #1049 seed-9 state, Wonder Tag establishes K1,
  // Earthen Vessel plus Vital Dance cover the Energy package, and Brilliant
  // Blender can put the Dragon payload in discard on T3. The profile identity
  // therefore cannot force the strict-JIT branch onto a duplicate Crispin /
  // second-Tapu route:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official turn, Item, Ability, Supporter, attachment, and evolution rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Exact reproduction and confirmed overfit: https://github.com/FlareZ123/pokemon-sims/issues/1049 https://github.com/FlareZ123/pokemon-sims/issues/2715
  for (const char* label :
       {"strict-jit/go-first", "matchup-flex-jit/go-first"}) {
    const SeedNineResult result = run_seed_nine(label);
    expect(result.t1_wonder_tags == 1,
           "issue-2715 spent a second T1 Wonder Tag on the deterministic route");
    expect(result.t1_wonder_tagged_steven,
           "issue-2715 failed to bank Steven on the exact seed-9 JIT route");
    expect(result.t1_held_second_tapu_for_steven_route,
           "issue-2715 failed to preserve the second Tapu after banking Steven");
    expect(result.first_ready_turn == 3,
           "issue-2715 lost the deterministic T3 ready turn");
  }
}

void test_no_discard_control_stays_outside_the_jit_projection() {
  // No-discard-control permits payload banking before the ready turn and therefore
  // remains outside this JIT-specific projection. The same seed must not acquire
  // the JIT-only Steven hold solely because the strict/matchup profile gate moved:
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Route scope: https://github.com/FlareZ123/pokemon-sims/issues/2715
  const SeedNineResult result = run_seed_nine("no-discard-control/go-first");
  expect(!result.t1_wonder_tagged_steven,
         "issue-2715 leaked the JIT-specific Steven projection into no-discard-control");
  expect(!result.t1_held_second_tapu_for_steven_route,
         "issue-2715 leaked the JIT-specific Tapu hold into no-discard-control");
}
}  // namespace

int main() {
  try {
    test_both_jit_profiles_use_the_same_seed_nine_route();
    test_no_discard_control_stays_outside_the_jit_projection();
    std::cout << "Issue 2715 Steven JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

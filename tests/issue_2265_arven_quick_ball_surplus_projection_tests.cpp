#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }

  static bool arven_quick_ball_projection_live(const Engine& engine) {
    return engine.issue_2265_arven_quick_ball_surplus_route_live();
  }

  static bool play_arven_quick_ball_projection(Engine& engine) {
    return engine.play_issue_2265_arven_quick_ball_surplus_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool turn_trace_contains(const sim::TraceLog& trace, const int turn,
                         const std::string& text) {
  const std::string prefix = "T" + std::to_string(turn) + " | ";
  for (const std::string& line : trace.lines) {
    if (line.find(prefix) != std::string::npos &&
        line.find(text) != std::string::npos) {
      return true;
    }
  }
  return false;
}

sim::TraceLog run_trace(const char* scenario_label, const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2265 fixture is unavailable.");

  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();
  return trace;
}

void test_strict_seed_122_uses_resource_preserving_t3_latias_finish() {
  const sim::TraceLog trace = run_trace("strict-jit/go-second", 122);

  // K1 shows the full T3 continuation before the first search Item is spent. The
  // correct order preserves Treasure through the scheduled Fire attachment and
  // evolution. At GGF, Treasure itself can search Psychic Latias ex by spending a
  // now route-replaced Supporter, then Blender supplies the JIT payload and Skyliner
  // promotes Regidrago VSTAR. This reaches the approved refined T3 deadline while
  // preserving Quick Ball, surplus Grass, Tapu Lele-GX, and Wonder Tag.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, discard, search, Bench, evolution, Ability, attachment, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, resource preservation, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Approved refinement: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
  expect(turn_trace_contains(trace, 3, "Fire Energy manually to Regidrago V."),
         "Strict seed 122 did not preserve the scheduled Fire attachment on T3.");
  expect(turn_trace_contains(trace, 3, "Evolved Regidrago V into Regidrago VSTAR."),
         "Strict seed 122 did not evolve before spending the Latias connector on T3.");
  expect(turn_trace_contains(trace, 3, "Searched a Psychic or Dragon Pokémon: Latias ex."),
         "Strict seed 122 did not use the shorter post-GGF Latias search on T3.");
  expect(!turn_trace_contains(trace, 3, "WONDER TAG") &&
             !turn_trace_contains(trace, 3, "Grass Energy (Quick Ball cost)"),
         "Strict seed 122 still spent the dominated Tapu/Wonder Tag or Quick Ball route on T3.");
  expect(turn_trace_contains(trace, 3, "READY"),
         "Strict seed 122 did not retain the approved T3 ready result.");
}

void test_strict_k1_state_still_projects_arven_when_treasure_is_absent() {
  sim::Scenario scenario{"issue-2265/projection", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2265};
  sim::Engine engine{scenario, recipe, rng};

  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::Arven, sim::Card::Grass,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::QuickBall, sim::Card::LatiasEx,
                sim::Card::Dragapult, sim::Card::Fire};
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));

  // This isolates the original underlying bug after the stronger held-Treasure
  // continuation is removed. Energy/evolution are complete, Quick Ball and Latias
  // are K1-known in deck, Grass is dynamically surplus, Blender is the surviving
  // current-turn payload outlet, and Arven has the unused Supporter permission.
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, Item, discard, search, Bench, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
  expect(sim::EngineTestAccess::arven_quick_ball_projection_live(engine),
         "The K1 Arven-to-Quick-Ball projection is still missing when Treasure is absent.");
  expect(sim::EngineTestAccess::play_arven_quick_ball_projection(engine),
         "The legal Arven-to-Quick-Ball projection could not resolve.");
}

void test_matchup_flex_seed_122_keeps_resource_preserving_t3_finish() {
  const sim::TraceLog trace = run_trace("matchup-flex-jit/go-second", 122);

  // Matchup-flex reaches the same T3 deadline and may use lower-DCI Lusamine as the
  // post-GGF Treasure cost. It should still avoid the redundant Tapu/Wonder Tag line.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Approved refinement: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
  expect(turn_trace_contains(trace, 3, "READY"),
         "Matchup-flex seed 122 regressed from its established T3 finish.");
  expect(!turn_trace_contains(trace, 3, "WONDER TAG"),
         "Matchup-flex seed 122 still spent the redundant Tapu/Wonder Tag branch on T3.");
}
}  // namespace

int main() {
  try {
    test_strict_seed_122_uses_resource_preserving_t3_latias_finish();
    test_strict_k1_state_still_projects_arven_when_treasure_is_absent();
    test_matchup_flex_seed_122_keeps_resource_preserving_t3_finish();
    std::cout << "Issue 2265 Arven Quick Ball projection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

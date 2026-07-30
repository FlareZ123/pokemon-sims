#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool duplicate_crispin_route(const Engine& engine) {
    return engine.issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
        false);
  }
  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};

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

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_seed(const std::string& scenario_label,
                    const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1516 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

sim::State issue_1922_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0};
  state.hand = {sim::Card::ChaoticSwell, sim::Card::QuickBall,
                sim::Card::Gladion, sim::Card::Crispin,
                sim::Card::ErikasInvitation, sim::Card::Grass,
                sim::Card::Fire};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::RegidragoVstar, sim::Card::Arven,
                sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
                sim::Card::Oricorio, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Guzma,
                  sim::Card::Powerglass, sim::Card::Channeler,
                  sim::Card::Dipplin, sim::Card::Arven};
  state.discard = {sim::Card::EarthenVessel, sim::Card::Dragapult};
  return state;
}

void test_k1_provenance_equivalence() {
  const auto route_is_live = [](const bool deck_seen,
                                const bool prizes_revealed,
                                const std::uint64_t seed) {
    const sim::Scenario scenario{
        "issue-1922", sim::DciProfile::NoDiscardControl,
        sim::LockMode::None, true, 5};
    const sim::DeckRecipe recipe = sim::baseline_recipe();
    std::mt19937_64 rng{seed};
    sim::Engine engine{scenario, recipe, rng};
    sim::EngineTestAccess::set_state(
        engine, issue_1922_state(), deck_seen, prizes_revealed);
    return sim::EngineTestAccess::duplicate_crispin_route(engine);
  };

  // Either legal inspection supplies the same K1 knowledge for the duplicate
  // Wonder Tag to Crispin projection. K0 cannot use the prized payload identity:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Official search, Ability, Item-cost, Prize, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1922
  expect(route_is_live(true, false, 192201),
         "Deck-search K1 rejected the duplicate-Crispin hold");
  expect(route_is_live(false, true, 192202),
         "Prize-inspection K1 rejected the duplicate-Crispin hold");
  expect(!route_is_live(false, false, 192203),
         "K0 used the duplicate-Crispin hold");
}

void test_seed_42_preserves_quick_ball_and_tapu() {
  const SeedResult result = run_seed("no-discard-control/go-first", 42);

  // Earthen Vessel establishes K1 and loads Dragapult ex. Held Gladion covers the
  // known prized Mega Dragonite ex, while held Crispin remains the Energy Supporter.
  // Quick Ball into Tapu and duplicate Crispin therefore spends physical resources
  // without improving the T3 deadline:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // No-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "Seed 42 lost its legal T3 ready turn.");
  expect(trace_contains(result.trace, "T1 | HOLD QUICK BALL"),
         "Seed 42 did not preserve the redundant Quick Ball route.");
  expect(!trace_contains(result.trace, "T1 | PLAY ITEM") &&
             !trace_contains(result.trace,
                             "T1 | BENCH | rules: R-GAME-BENCH | Tapu Lele-GX") &&
             !trace_contains(result.trace, "T1 | WONDER TAG"),
         "Seed 42 still spent Quick Ball and Tapu on duplicate Crispin.");
  expect(trace_contains(result.trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(result.trace, "Gladion") &&
             trace_contains(result.trace, "T3 | READY"),
         "Seed 42 did not retain the Gladion-to-Crispin T3 continuation.");
}

void test_strict_jit_seed_104_keeps_distinct_tapu_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 104);

  // Issue #1552 preserves the distinct Tapu Lele-GX to Crispin connector and
  // advances it one turn. T1 Vessel establishes K1, then T2 Quick Ball places the
  // strict-JIT Dragon payload while Tapu and Crispin complete the Energy axis:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Existing connector boundary: https://github.com/FlareZ123/pokemon-sims/issues/962
  // Confirmed faster route: https://github.com/FlareZ123/pokemon-sims/issues/1552
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Strict-JIT seed 104 lost its T2 route.");
  expect(trace_contains(result.trace,
                        "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(result.trace, "WONDER TAG") &&
             trace_contains(result.trace, "Crispin") &&
             trace_contains(result.trace, "T2 | READY"),
         "Strict-JIT seed 104 lost its faster Tapu-Crispin connector.");
}

}  // namespace

int main() {
  test_k1_provenance_equivalence();
  test_seed_42_preserves_quick_ball_and_tapu();
  test_strict_jit_seed_104_keeps_distinct_tapu_route();
  return 0;
}

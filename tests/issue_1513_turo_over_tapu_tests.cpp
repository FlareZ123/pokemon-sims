#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool held_turo_route(const Engine& engine) {
    return engine.held_turo_direct_active_promotion_route();
  }

  static bool bench_tapu(Engine& engine) {
    return engine.bench_tapu_if_useful();
  }

  static bool play_turo(Engine& engine) {
    return engine.play_turo_active_promotion_route();
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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&text](const std::string& line) {
    return line.find(text) != std::string::npos;
  });
}

sim::State exact_t3_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::ForestSealStone},
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {
      sim::Card::TapuLeleGX,
      sim::Card::ProfessorTuro,
      sim::Card::Fire,
      sim::Card::BrilliantBlender};
  state.discard = {
      sim::Card::MysteriousTreasure,
      sim::Card::Dragapult,
      sim::Card::Crispin};
  return state;
}

void test_exact_state_preserves_tapu_and_uses_direct_turo() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr, "The issue-1513 fixture is unavailable.");

  std::mt19937_64 rng(151301);
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_t3_state());

  // Turo directly returns the Basic Active and the complete Benched VSTAR becomes
  // Active. Tapu -> Wonder Tag -> Tate spends extra discrete resources for the same
  // axis: https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1513
  expect(sim::EngineTestAccess::held_turo_route(engine),
         "The direct held-Turo route was not recognized.");
  expect(!sim::EngineTestAccess::bench_tapu(engine),
         "Tapu Lele-GX was still Benched before the direct Turo route.");
  expect(sim::EngineTestAccess::play_turo(engine),
         "Professor Turo did not execute the direct Active-promotion route.");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.active && state.active->card == sim::Card::RegidragoVstar,
         "The complete Benched Regidrago VSTAR was not promoted.");
  expect(contains(state.hand, sim::Card::DialgaGX),
         "The Basic Active was not returned to hand.");
  expect(contains(state.hand, sim::Card::TapuLeleGX),
         "Tapu Lele-GX was not preserved in hand.");
  expect(contains(state.discard, sim::Card::ProfessorTuro),
         "Professor Turo was not consumed as the Supporter action.");
  expect(!state.retreat_used,
         "The direct Turo route incorrectly consumed the Retreat action.");
  expect(std::none_of(state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
           return pokemon.card == sim::Card::TapuLeleGX;
         }),
         "Tapu Lele-GX remained as an avoidable two-Prize Bench liability.");
}

void test_route_boundaries_preserve_existing_choices() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr, "The boundary fixture is unavailable.");

  auto route_from = [&](sim::State state, const std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    sim::Engine engine(*scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    return sim::EngineTestAccess::held_turo_route(engine);
  };

  sim::State no_turo = exact_t3_state();
  no_turo.hand.erase(std::remove(no_turo.hand.begin(), no_turo.hand.end(), sim::Card::ProfessorTuro),
                     no_turo.hand.end());
  expect(!route_from(std::move(no_turo), 151302),
         "The route was admitted without Professor Turo.");

  sim::State held_tate = exact_t3_state();
  held_tate.hand.push_back(sim::Card::TateLiza);
  expect(!route_from(std::move(held_tate), 151303),
         "The existing naturally held Tate & Liza preference was displaced.");

  sim::State incomplete_vstar = exact_t3_state();
  incomplete_vstar.bench.front().grass = 1;
  expect(!route_from(std::move(incomplete_vstar), 151304),
         "The route was admitted without a complete Benched GGF VSTAR.");

  sim::State free_retreat = exact_t3_state();
  free_retreat.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 1});
  expect(!route_from(std::move(free_retreat), 151305),
         "Turo displaced an already-live zero-Retreat Latias route.");
}

void test_seed_47_uses_turo_without_tapu_tate_chain() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr, "The seed-47 fixture is unavailable.");

  std::mt19937_64 rng(47);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound seed must keep its earliest T3 readiness while avoiding the
  // UDP Tapu, Wonder Tag, and Tate chain: https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1513
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 47 did not retain its earliest T3 ready state.");
  expect(trace_contains(trace, "T3 | HOLD TAPU LELE-GX") &&
             trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "Professor Turo returned the Basic Active Pokémon"),
         "Seed 47 did not use the preserved-Tapu direct Turo route.");
  expect(!trace_contains(trace, "T3 | BENCH | Tapu Lele-GX") &&
             !trace_contains(trace, "T3 | WONDER TAG") &&
             !trace_contains(trace, "T3 | PLAY SUPPORTER | Tate & Liza"),
         "Seed 47 still used the longer Tapu -> Tate & Liza chain.");

  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(state.active && state.active->card == sim::Card::RegidragoVstar,
         "Seed 47 did not finish with the complete Regidrago VSTAR Active.");
  expect(contains(state.hand, sim::Card::DialgaGX),
         "Seed 47 did not return Dialga-GX to hand.");
  expect(contains(state.hand, sim::Card::TapuLeleGX),
         "Seed 47 did not preserve Tapu Lele-GX in hand.");
  expect(!state.retreat_used,
         "Seed 47 incorrectly consumed the Retreat action.");
}

}  // namespace

int main() {
  const int digest_status = std::system(
      "python3 -c \"from pathlib import Path; import sys; "
      "root=Path('..').resolve(); sys.path.insert(0,str(root)); "
      "from scripts.baseline_provenance import simulator_policy_source_digest; "
      "print('ISSUE1513_SOURCE_DIGEST='+simulator_policy_source_digest(root))\"");
  expect(digest_status == 0, "The source-digest probe failed.");
  test_exact_state_preserves_tapu_and_uses_direct_turo();
  test_route_boundaries_preserve_existing_choices();
  test_seed_47_uses_turo_without_tapu_tate_chain();
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State base_state() {
  sim::State state;
  state.turn = 1;
  state.hand = {sim::Card::HisuianHeavyBall};
  state.deck = {sim::Card::Grass};
  state.prizes = {sim::Card::Pineco, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Powerglass, sim::Card::FieldBlower,
                  sim::Card::Grant};
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::LockMode lock, const std::uint64_t seed, sim::State state)
      : scenario{"issue-1964", sim::DciProfile::StrictJit, lock, false, 4},
        rng(seed),
        engine(scenario, [] {
          sim::DeckRecipe recipe = sim::baseline_recipe();
          recipe.emplace_back(sim::Card::Pineco, 1);
          return recipe;
        }(), rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state));
  }
};

void expect_pineco_exchange(sim::Engine& engine, const char* message) {
  expect(sim::EngineTestAccess::play_heavy_ball(engine), message);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  // Hisuian Heavy Ball may exchange itself for any revealed Basic Pokemon.
  // Pineco is a Basic Pokemon and therefore remains a legal exhaustive fallback:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1964
  expect(contains(after.hand, sim::Card::Pineco) &&
             !contains(after.prizes, sim::Card::Pineco) &&
             contains(after.prizes, sim::Card::HisuianHeavyBall) &&
             !contains(after.discard, sim::Card::HisuianHeavyBall),
         "Hisuian Heavy Ball did not exchange for prized Pineco.");
}

void test_exhaustive_basic_fallback_recovers_pineco() {
  Fixture fixture(sim::LockMode::None, 1964, base_state());
  expect_pineco_exchange(fixture.engine,
      "The exhaustive Heavy Ball fallback omitted Pineco.");
}

void test_full_bench_does_not_remove_pineco_from_printed_targets() {
  sim::State state = base_state();
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::LatiasEx, 1},
  };
  Fixture fixture(sim::LockMode::None, 1965, std::move(state));
  expect_pineco_exchange(fixture.engine,
      "A full Bench incorrectly removed Pineco from Heavy Ball's targets.");
}

void test_stronger_regidrago_axis_keeps_priority() {
  sim::State state = base_state();
  state.prizes[1] = sim::Card::RegidragoV;
  Fixture fixture(sim::LockMode::None, 1966, std::move(state));
  expect(sim::EngineTestAccess::play_heavy_ball(fixture.engine),
         "The stronger Regidrago Heavy Ball route did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.prizes, sim::Card::Pineco),
         "Pineco displaced the stronger missing-Regidrago axis.");
}

void test_item_lock_preserves_the_existing_legality_gate() {
  Fixture fixture(sim::LockMode::FullItem, 1967, base_state());
  // Item lock prevents Heavy Ball from being played, so the action helper returns
  // false and every zone remains unchanged:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
  const bool played = sim::EngineTestAccess::play_heavy_ball(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(!played &&
             contains(after.hand, sim::Card::HisuianHeavyBall) &&
             contains(after.prizes, sim::Card::Pineco) &&
             !contains(after.hand, sim::Card::Pineco),
         "The Item-lock boundary mutated the Heavy Ball state.");
}
}  // namespace

int main() {
  try {
    test_exhaustive_basic_fallback_recovers_pineco();
    test_full_bench_does_not_remove_pineco_from_printed_targets();
    test_stronger_regidrago_axis_keeps_priority();
    test_item_lock_preserves_the_existing_legality_gate();
    std::cout << "Issue 1964 Heavy Ball Pineco tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

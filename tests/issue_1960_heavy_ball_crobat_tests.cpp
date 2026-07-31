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
  state.prizes = {sim::Card::CrobatV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Powerglass, sim::Card::FieldBlower,
                  sim::Card::Grant};
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::LockMode lock, const std::uint64_t seed, sim::State state)
      : scenario{"issue-1960", sim::DciProfile::StrictJit, lock, false, 4},
        rng(seed),
        engine(scenario, [] {
          sim::DeckRecipe recipe = sim::baseline_recipe();
          recipe.emplace_back(sim::Card::CrobatV, 1);
          return recipe;
        }(), rng) {
    sim::EngineTestAccess::set_state(engine, std::move(state));
  }
};

void expect_crobat_exchange(sim::Engine& engine, const char* message) {
  expect(sim::EngineTestAccess::play_heavy_ball(engine), message);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  // Heavy Ball may exchange itself for any revealed Basic Pokemon. Crobat V is a
  // Basic Pokemon V, so every legal fallback must preserve the printed exchange:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1960
  expect(contains(after.hand, sim::Card::CrobatV) &&
             !contains(after.prizes, sim::Card::CrobatV) &&
             contains(after.prizes, sim::Card::HisuianHeavyBall) &&
             !contains(after.discard, sim::Card::HisuianHeavyBall),
         "Hisuian Heavy Ball did not exchange for prized Crobat V.");
}

void test_immediate_dark_asset_connector_priority() {
  sim::State state = base_state();
  state.prizes[1] = sim::Card::DialgaGX;
  Fixture fixture(sim::LockMode::None, 1960, std::move(state));
  expect_crobat_exchange(fixture.engine,
      "The immediate Crobat V Heavy Ball connector did not resolve.");
}

void test_full_bench_still_uses_exhaustive_basic_fallback() {
  sim::State state = base_state();
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::LatiasEx, 1},
  };
  Fixture fixture(sim::LockMode::None, 1961, std::move(state));
  expect_crobat_exchange(fixture.engine,
      "A full Bench incorrectly removed Crobat V from Heavy Ball's Basic targets.");
}

void test_inert_dark_asset_states_still_recover_the_basic() {
  for (const bool used_dark_asset : {false, true}) {
    for (const bool empty_deck : {false, true}) {
      const std::uint64_t seed = 1962U +
          static_cast<unsigned>(used_dark_asset) * 2U +
          static_cast<unsigned>(empty_deck);
      sim::State state = base_state();
      state.dark_asset_used = used_dark_asset;
      if (empty_deck) state.deck.clear();
      state.hand.insert(state.hand.end(), 7, sim::Card::Grass);
      Fixture fixture(sim::LockMode::FullRuleBoxAbility, seed, std::move(state));
      expect_crobat_exchange(fixture.engine,
          "An inert Dark Asset state removed Crobat V from the exhaustive fallback.");
    }
  }
}

void test_stronger_missing_regidrago_axis_keeps_priority() {
  sim::State state = base_state();
  state.prizes[1] = sim::Card::RegidragoV;
  Fixture fixture(sim::LockMode::None, 1966, std::move(state));
  expect(sim::EngineTestAccess::play_heavy_ball(fixture.engine),
         "The stronger Regidrago Heavy Ball route did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.prizes, sim::Card::CrobatV),
         "Crobat V displaced the stronger missing-Regidrago axis.");
}
}  // namespace

int main() {
  try {
    test_immediate_dark_asset_connector_priority();
    test_full_bench_still_uses_exhaustive_basic_fallback();
    test_inert_dark_asset_states_still_recover_the_basic();
    test_stronger_missing_regidrago_axis_keeps_priority();
    std::cout << "Issue 1960 Heavy Ball Crobat V tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

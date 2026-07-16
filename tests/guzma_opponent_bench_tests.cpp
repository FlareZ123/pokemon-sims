#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"guzma-opponent-bench", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{741};
  sim::Engine engine{scenario, recipe, rng};

  explicit Fixture(const sim::OpponentBenchState opponent_bench)
      : scenario{"guzma-opponent-bench", sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 4, opponent_bench},
        engine{scenario, recipe, rng} {}
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

sim::State complete_guzma_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::Guzma};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect_guzma_held(const sim::OpponentBenchState opponent_bench,
                       const std::string_view label) {
  Fixture fixture(opponent_bench);
  sim::State state = complete_guzma_state();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::Oricorio,
         std::string(label) + ": Guzma must not change the Active Pokémon.");
  expect(contains(after.hand, sim::Card::Guzma) && !after.supporter_used,
         std::string(label) + ": Guzma and the Supporter play must be preserved.");
}

void test_unknown_opponent_bench_holds_guzma() {
  // Guzma's self-switch occurs only if the opponent's switch was performed. The
  // default goldfish state has no opposing board and cannot prove that prerequisite:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  expect_guzma_held(sim::OpponentBenchState::Unknown, "unknown opponent Bench");
}

void test_unavailable_opponent_bench_holds_guzma() {
  // An explicitly empty or otherwise ineligible opposing Bench makes Guzma's first
  // switch impossible, so the conditional self-switch cannot resolve:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  expect_guzma_held(sim::OpponentBenchState::Unavailable,
                    "unavailable opponent Bench");
}

void test_available_opponent_bench_resolves_guzma() {
  Fixture fixture(sim::OpponentBenchState::Available);
  sim::State state = complete_guzma_state();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  // The explicit Available scenario proves that the opponent-side switch can be
  // performed. Guzma may then switch the GGF-ready Benched Regidrago VSTAR Active:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Available opponent Bench must permit Guzma promotion.");
  expect(after.supporter_used && contains(after.discard, sim::Card::Guzma) &&
             !contains(after.hand, sim::Card::Guzma),
         "Resolved Guzma must consume the Supporter play and enter discard.");
}

void test_missing_payload_holds_guzma() {
  Fixture fixture(sim::OpponentBenchState::Available);
  sim::State state = complete_guzma_state();
  state.discard.clear();
  state.discarded_this_turn.clear();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  // Promotion alone does not complete strict-JIT readiness when no present-turn
  // Dragon payload exists. Preserve Guzma for a turn with a complete route:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::Oricorio &&
             contains(after.hand, sim::Card::Guzma) && !after.supporter_used,
         "Guzma must be held while the strict-JIT payload axis is missing.");
}

void test_incomplete_energy_holds_guzma() {
  Fixture fixture(sim::OpponentBenchState::Available);
  sim::State state = complete_guzma_state();
  state.bench.front().grass = 1;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  // Apex Dragon requires GGF. Guzma must not spend the Supporter play to promote an
  // Energy-incomplete attacker when that action cannot finish readiness:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::Oricorio &&
             contains(after.hand, sim::Card::Guzma) && !after.supporter_used,
         "Guzma must be held for an Energy-incomplete Benched VSTAR.");
}

void test_tate_remains_preferred() {
  Fixture fixture(sim::OpponentBenchState::Available);
  sim::State state = complete_guzma_state();
  state.hand.push_back(sim::Card::TateLiza);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  // Tate & Liza provides the unconditional self-switch that the engine already
  // models. Preserve the discrete-value Guzma when Tate completes the same route:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Tate & Liza must still promote the complete VSTAR.");
  expect(contains(after.hand, sim::Card::Guzma) &&
             contains(after.discard, sim::Card::TateLiza),
         "Tate & Liza must be preferred while Guzma remains held.");
}

void test_guzma_plus_blender_route() {
  Fixture fixture(sim::OpponentBenchState::Available);
  sim::State state = complete_guzma_state();
  state.discard.clear();
  state.discarded_this_turn.clear();
  state.hand.push_back(sim::Card::BrilliantBlender);
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Brilliant Blender supplies the present-turn Dragon payload, then the explicit
  // opponent-Bench assumption permits Guzma's conditional self-switch:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender must establish the live payload route.");
  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Guzma plus Blender must complete the Active axis.");
  expect(after.supporter_used && contains(after.discard, sim::Card::Guzma) &&
             contains(after.discarded_this_turn, sim::Card::MegaDragonite),
         "The combined route must consume Guzma and retain a present-turn payload.");
}

}  // namespace

int main() {
  try {
    test_unknown_opponent_bench_holds_guzma();
    test_unavailable_opponent_bench_holds_guzma();
    test_available_opponent_bench_resolves_guzma();
    test_missing_payload_holds_guzma();
    test_incomplete_energy_holds_guzma();
    test_tate_remains_preferred();
    test_guzma_plus_blender_route();
    std::cout << "guzma opponent-Bench regressions passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_known_prize_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.prizes_revealed_ = true;
  }

  static bool play_heavy_ball(Engine& engine) {
    return engine.play_heavy_ball();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t heavy_ball_hold_count(const sim::TraceLog& trace) {
  return static_cast<std::size_t>(std::count_if(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("Known Prize cards contain no Basic Pokemon") !=
               std::string::npos;
      }));
}

void test_seed_69_emits_one_hold_for_the_unchanged_state() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{69};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  engine.run();

  // K1 proves that Heavy Ball has no Basic Prize target, so retaining the Item is
  // legal. Fixed-point re-evaluation must expose one decision for the unchanged state:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // https://github.com/FlareZ123/pokemon-sims/issues/1007
  expect(heavy_ball_hold_count(trace) == 1U,
         "Seed 69 must emit exactly one unchanged-state Heavy Ball hold event.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::HisuianHeavyBall) == 1,
         "The trace fix must preserve Heavy Ball as a held discard-cost resource.");
}

void test_state_change_allows_a_new_hold_event() {
  const sim::Scenario scenario{"issue-1007", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1007};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);

  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::HisuianHeavyBall};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Arven,
                  sim::Card::Crispin, sim::Card::ForestSealStone,
                  sim::Card::MysteriousTreasure};
  sim::EngineTestAccess::set_known_prize_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_heavy_ball(engine),
         "Known-no-Basic Heavy Ball must remain held.");
  expect(!sim::EngineTestAccess::play_heavy_ball(engine),
         "Repeated stabilization must still re-evaluate the predicate.");
  expect(heavy_ball_hold_count(trace) == 1U,
         "An unchanged exact state must emit one hold event.");

  engine.state_.hand.push_back(sim::Card::Grass);
  expect(!sim::EngineTestAccess::play_heavy_ball(engine),
         "Heavy Ball must remain held after the relevant state changes.");
  expect(heavy_ball_hold_count(trace) == 2U,
         "A changed exact state must be allowed to emit a new hold event.");
}
}  // namespace

int main() {
  try {
    test_seed_69_emits_one_hold_for_the_unchanged_state();
    test_state_change_allows_a_new_hold_event();
    std::cout << "Issue 1007 Heavy Ball hold trace tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

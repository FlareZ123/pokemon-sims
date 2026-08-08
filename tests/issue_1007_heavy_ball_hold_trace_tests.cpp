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

  static void add_to_hand(Engine& engine, const Card card) {
    engine.state_.hand.push_back(card);
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

void test_seed_69_stops_before_the_obsolete_later_heavy_ball_state() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{69};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  // Issue #2164 now completes seed 69 on T4 through Quick Ball -> Latias ex, so
  // simulation terminates before the later turn that used to expose Heavy Ball.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Current-turn JIT / earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Earlier Heavy Ball trace contract: https://github.com/FlareZ123/pokemon-sims/issues/1007
  // Confirmed route improvement: https://github.com/FlareZ123/pokemon-sims/issues/2164
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 69 must terminate on the corrected T4 finish.");
  expect(heavy_ball_hold_count(trace) == 0U,
         "Seed 69 must not manufacture a later Heavy Ball state after T4 readiness.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::HisuianHeavyBall) == 0,
         "Heavy Ball must remain undrawn when seed 69 terminates on T4.");
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

  // The original issue-1007 exact-state invariant remains independently covered:
  // K1 proves that Heavy Ball has no Basic Prize target, so preserving it is legal.
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1007
  expect(!sim::EngineTestAccess::play_heavy_ball(engine),
         "Known-no-Basic Heavy Ball must remain held.");
  expect(!sim::EngineTestAccess::play_heavy_ball(engine),
         "Repeated stabilization must still re-evaluate the predicate.");
  expect(heavy_ball_hold_count(trace) == 1U,
         "An unchanged exact state must emit one hold event.");

  sim::EngineTestAccess::add_to_hand(engine, sim::Card::Grass);
  expect(!sim::EngineTestAccess::play_heavy_ball(engine),
         "Heavy Ball must remain held after the relevant state changes.");
  expect(heavy_ball_hold_count(trace) == 2U,
         "A changed exact state must be allowed to emit a new hold event.");
}
}  // namespace

int main() {
  try {
    test_seed_69_stops_before_the_obsolete_later_heavy_ball_state();
    test_state_change_allows_a_new_hold_event();
    std::cout << "Issue 1007 Heavy Ball hold trace tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

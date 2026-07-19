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
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool use_fss(Engine& engine) {
    return engine.use_fss();
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

std::size_t k1_fss_hold_count(const sim::TraceLog& trace) {
  return static_cast<std::size_t>(std::count_if(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("K1 proved that no searched card advances setup") !=
               std::string::npos;
      }));
}

void test_seed_46_emits_one_k1_hold_for_the_unchanged_state() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{46};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Forest Seal Stone grants one Star Alchemy use to its Pokémon V holder. K1 may
  // preserve it when no searched card advances setup, while repeated stabilization
  // must expose one decision for the unchanged state:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // https://github.com/FlareZ123/pokemon-sims/issues/1010
  expect(k1_fss_hold_count(trace) == 1U,
         "Seed 46 must emit exactly one unchanged-state K1 Star Alchemy hold event.");
  expect(outcome.ready_by_4 && outcome.first_ready_turn == 4,
         "The trace fix must preserve seed 46's legal T4 setup route.");
}

void test_state_change_allows_a_new_k1_hold_event() {
  const sim::Scenario scenario{"issue-1010", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1010};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::RegidragoVstar,
                sim::Card::StevensResolve,
                sim::Card::Channeler,
                sim::Card::GoodraVstar};
  state.deck = {sim::Card::Crispin,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::HisuianHeavyBall,
                  sim::Card::Grass,
                  sim::Card::PathToPeak,
                  sim::Card::ErikasInvitation,
                  sim::Card::QuickBall,
                  sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::use_fss(engine),
         "The K1 no-advancing-target state must retain Star Alchemy.");
  expect(!sim::EngineTestAccess::use_fss(engine),
         "Repeated stabilization must still evaluate Star Alchemy.");
  expect(k1_fss_hold_count(trace) == 1U,
         "An unchanged K1 state must emit one Star Alchemy hold event.");
  expect(!engine.state().vstar_power_used,
         "Retaining Star Alchemy must leave the once-per-game power unused.");

  sim::EngineTestAccess::add_to_hand(engine, sim::Card::Channeler);
  expect(!sim::EngineTestAccess::use_fss(engine),
         "A changed state may still retain Star Alchemy.");
  expect(k1_fss_hold_count(trace) == 2U,
         "A changed state must be allowed to emit a new hold event.");
}
}  // namespace

int main() {
  try {
    test_seed_46_emits_one_k1_hold_for_the_unchanged_state();
    test_state_change_allows_a_new_k1_hold_event();
    std::cout << "Issue 1010 Forest Seal Stone hold trace tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

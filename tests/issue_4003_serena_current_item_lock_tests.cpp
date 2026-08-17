#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool item_locked(const Engine& engine) { return engine.item_locked(); }
  static bool play_permanent_item_serena(Engine& engine) {
    return engine.play_serena_permanent_item_lock_if_selected(false);
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

sim::State serena_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Serena, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                sim::Card::Channeler, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::LatiasEx};
  return state;
}

sim::Engine make_engine(const sim::LockMode locks, const int turn,
                        std::mt19937_64& rng) {
  const sim::Scenario scenario{"issue-4003", sim::DciProfile::StrictJit,
                               locks, false, 4};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, serena_state(turn));
  return engine;
}

void test_combined_lock_uses_current_turn_item_window() {
  // FullCombined preserves legal Item play on the player's first turn and begins
  // its Item restriction on turn two. Serena's optional-discard optimization may
  // therefore promote locked Items only once the canonical current-turn predicate
  // says the Item action is actually prohibited:
  // Canonical Item-lock primitive: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Combined-lock timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#combined-lock
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Advanced Item/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4003
  std::mt19937_64 turn_one_rng{400301};
  sim::Engine turn_one =
      make_engine(sim::LockMode::FullCombined, 1, turn_one_rng);
  expect(!sim::EngineTestAccess::item_locked(turn_one),
         "FullCombined incorrectly Item-locks turn one.");
  expect(!sim::EngineTestAccess::play_permanent_item_serena(turn_one),
         "Turn-one combined lock entered the permanent-Item Serena optimizer.");
  expect(contains(turn_one.state().hand, sim::Card::EarthenVessel),
         "Rejected turn-one optimizer mutated a live Item.");

  std::mt19937_64 turn_two_rng{400302};
  sim::Engine turn_two =
      make_engine(sim::LockMode::FullCombined, 2, turn_two_rng);
  expect(sim::EngineTestAccess::item_locked(turn_two),
         "FullCombined did not Item-lock turn two.");
  expect(sim::EngineTestAccess::play_permanent_item_serena(turn_two),
         "Turn-two combined lock lost the established Serena optimizer.");
  expect(contains(turn_two.state().discard, sim::Card::EarthenVessel),
         "Turn-two optimizer did not spend a currently locked Item.");
}

void test_full_item_lock_remains_active_on_turn_one() {
  // FullItem is the explicit always-on Item-lock fixture, so the same optimization
  // remains available on turn one when the player is otherwise allowed a Supporter:
  // Full Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/4003
  std::mt19937_64 rng{400303};
  sim::Engine engine = make_engine(sim::LockMode::FullItem, 1, rng);
  expect(sim::EngineTestAccess::item_locked(engine),
         "FullItem unexpectedly opened the turn-one Item window.");
  expect(sim::EngineTestAccess::play_permanent_item_serena(engine),
         "FullItem lost the established turn-one Serena optimizer.");
}
}  // namespace

int main() {
  try {
    test_combined_lock_uses_current_turn_item_window();
    test_full_item_lock_remains_active_on_turn_one();
    std::cout << "Issue 4003 Serena current-Item-lock tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

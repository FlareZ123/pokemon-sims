#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool value) { engine.deck_seen_ = value; }
  static bool run_search_items_one_step(Engine& engine) {
    return engine.run_search_items_one_step(true);
  }
  static std::optional<int> delayed_vessel_ready_turn(const Engine& engine) {
    return engine.issue_1447_vessel_ready_turn_;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe fixture_recipe() {
  return {
      {sim::Card::RegidragoVstar, 1},
      {sim::Card::EarthenVessel, 1},
      {sim::Card::Crispin, 1},
      {sim::Card::DialgaGX, 1},
      {sim::Card::Grass, 2},
      {sim::Card::Fire, 1},
  };
}

sim::State hold_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn - 1, 0, 0,
                              sim::Tool::None, 0};
  state.hand = {sim::Card::EarthenVessel, sim::Card::Crispin,
                sim::Card::DialgaGX};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State ready_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, turn - 2, 1, 1,
                              sim::Tool::None, 0};
  state.hand = {sim::Card::EarthenVessel, sim::Card::DialgaGX};
  state.deck = {sim::Card::Grass};
  return state;
}

void expect_relative_hold(const int source_turn, const int max_turn) {
  const sim::Scenario scenario{"issue-3561-relative-hold", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, max_turn};
  std::mt19937_64 rng(static_cast<std::uint64_t>(356100 + source_turn));
  sim::Engine engine(scenario, fixture_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, hold_state(source_turn));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // #1447 is a current-turn versus immediately-following-turn resource comparison.
  // Earthen Vessel and Crispin impose no absolute T2/T3 restriction, so the accepted
  // T2->T3 witness and an equivalent T3->T4 state must record the same +1 continuation:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item, Supporter, attachment, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Original accepted route: https://github.com/FlareZ123/pokemon-sims/issues/1447
  // Relative-turn correction: https://github.com/FlareZ123/pokemon-sims/issues/3561
  const bool acted = sim::EngineTestAccess::run_search_items_one_step(engine);
  expect(!acted, "The delayed-Vessel hold must preserve Earthen Vessel this turn.");
  expect(sim::EngineTestAccess::delayed_vessel_ready_turn(engine) == source_turn + 1,
         "The delayed-Vessel hold did not record the immediately following turn.");
  expect(std::count(sim::EngineTestAccess::state(engine).hand.begin(),
                    sim::EngineTestAccess::state(engine).hand.end(),
                    sim::Card::EarthenVessel) == 1,
         "The delayed-Vessel hold consumed Earthen Vessel.");
}

void expect_recorded_continuation_plays(const int source_turn, const int max_turn) {
  const sim::Scenario scenario{"issue-3561-relative-finish", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, max_turn};
  std::mt19937_64 rng(static_cast<std::uint64_t>(356200 + source_turn));
  sim::Engine engine(scenario, fixture_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, hold_state(source_turn));
  sim::EngineTestAccess::set_deck_seen(engine, true);
  expect(!sim::EngineTestAccess::run_search_items_one_step(engine),
         "The source state did not hold Vessel.");

  const int ready_turn = source_turn + 1;
  sim::EngineTestAccess::set_state(engine, ready_state(ready_turn));

  // The recorded immediately-following turn must resolve the held Vessel rather
  // than applying the generalized hold predicate again. Vessel discards the Dragon
  // payload and searches the final Basic Energy for Apex Dragon's GGF cost:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Original accepted route: https://github.com/FlareZ123/pokemon-sims/issues/1447
  // Relative-turn correction: https://github.com/FlareZ123/pokemon-sims/issues/3561
  expect(sim::EngineTestAccess::run_search_items_one_step(engine),
         "The recorded next-turn continuation did not play Earthen Vessel.");
  expect(std::count(sim::EngineTestAccess::state(engine).discard.begin(),
                    sim::EngineTestAccess::state(engine).discard.end(),
                    sim::Card::DialgaGX) == 1,
         "The recorded continuation did not discard the held Dragon payload.");
}

void test_t2_to_t3_witness_remains_relative() {
  expect_relative_hold(2, 3);
  expect_recorded_continuation_plays(2, 3);
}

void test_equivalent_t3_to_t4_state_is_admitted() {
  expect_relative_hold(3, 4);
  expect_recorded_continuation_plays(3, 4);
}

void test_exhausted_horizon_does_not_hold() {
  const sim::Scenario scenario{"issue-3561-horizon", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng(356105);
  sim::Engine engine(scenario, fixture_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, hold_state(4));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // A delayed route cannot be selected without an immediately following simulated
  // turn. This is a simulator-horizon constraint, independent of card legality:
  // Repository route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Relative-turn correction: https://github.com/FlareZ123/pokemon-sims/issues/3561
  sim::EngineTestAccess::run_search_items_one_step(engine);
  expect(!sim::EngineTestAccess::delayed_vessel_ready_turn(engine).has_value(),
         "The exhausted-horizon state incorrectly banked a next-turn continuation.");
}

}  // namespace

int main() {
  test_t2_to_t3_witness_remains_relative();
  test_equivalent_t3_to_t4_state_is_admitted();
  test_exhausted_horizon_does_not_hold();
}

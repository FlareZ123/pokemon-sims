#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool retreat_for_celestial_roar(Engine& engine) {
    return engine.retreat_to_benched_regidrago_v_for_celestial_roar_with_latias();
  }
  static bool use_celestial_roar(Engine& engine) { return engine.use_celestial_roar(); }
  static bool attach_powerglass(Engine& engine) { return engine.attach_powerglass(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_live_route_promotes_and_attacks() {
  const sim::Scenario scenario{"issue-1325/live", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1325001};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None}};
  state.deck = {sim::Card::MysteriousTreasure, sim::Card::RegidragoVstar,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // Skyliner removes the Basic Active's Retreat Cost. The projected Regidrago V
  // already pays Celestial Roar's one-Colorless cost, and K1 proves that a needed
  // Fire Energy remains in the attack's top-three window:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1325
  expect(sim::EngineTestAccess::retreat_for_celestial_roar(engine),
         "A live Celestial Roar route must promote the energized Benched Regidrago V.");
  const sim::State& promoted = sim::EngineTestAccess::state(engine);
  expect(promoted.active && promoted.active->card == sim::Card::RegidragoV &&
             promoted.active->grass == 2 && promoted.retreat_used,
         "Skyliner promotion must preserve the energized Regidrago V and consume Retreat.");
  expect(sim::EngineTestAccess::use_celestial_roar(engine),
         "The promoted Regidrago V must use the already-live Celestial Roar policy.");
  const sim::State& attacked = sim::EngineTestAccess::state(engine);
  expect(attacked.active && attacked.active->fire == 1 && attacked.turn_ended,
         "Celestial Roar must attach the known Fire Energy and end the attack turn.");
}

void test_hold_gate_preserves_latias_and_retreat() {
  const sim::Scenario scenario{"issue-1325/hold", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1325002};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None}};
  state.deck = {sim::Card::TateLiza};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // Strict JIT cannot bank an early payload, and a GGF Regidrago V needs no Energy.
  // Reusing the exact attack gate must therefore hold both Celestial Roar and Retreat:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1325
  expect(!sim::EngineTestAccess::retreat_for_celestial_roar(engine),
         "A held Celestial Roar policy must not spend Skyliner Retreat.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::LatiasEx &&
             !after.retreat_used,
         "The hold gate must preserve Active Latias ex and the Retreat action.");
}

void test_rule_box_lock_blocks_skyliner_route() {
  const sim::Scenario scenario{"issue-1325/lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1325003};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None}};
  state.deck = {sim::Card::Fire, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // Path-to-the-Peak-style Rule Box Ability lock suppresses Skyliner:
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1325
  expect(!sim::EngineTestAccess::retreat_for_celestial_roar(engine),
         "Rule Box Ability lock must suppress the Skyliner promotion.");
}

void test_live_route_precedes_powerglass_attachment() {
  const sim::Scenario scenario{"issue-1325/powerglass", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1325004};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::Powerglass};
  state.discard = {sim::Card::Fire};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // The final Retreat must resolve before Powerglass so its effect remains attached
  // to the Active Regidrago V that neds the discarded Fire Energy:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://github.com/FlareZ123/pokemon-sims/issues/1325
  expect(sim::EngineTestAccess::retreat_for_celestial_roar(engine),
         "The live attack route must promote Regidrago V before Tool attachment.");
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "Powerglass must attach to the promoted Active Regidrago V.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoV &&
             after.active->tool == sim::Tool::Powerglass,
         "Powerglass ordering must preserve the promoted Regidrago V as its holder.");
}


void test_no_discard_control_preserves_deterministic_continuation() {
  const sim::Scenario scenario{"issue-1325/no-control", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1325005};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                               sim::Tool::None}};
  state.discard = {sim::Card::GoodraVstar};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // No-discard-control may bank its payload before the ready turn. Preserve its
  // established deterministic continuation rather than introducing a random
  // Celestial Roar promotion after the payload axis is already complete:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/issues/1325
  expect(!sim::EngineTestAccess::retreat_for_celestial_roar(engine),
         "The JIT-only promotion must preserve no-discard-control continuation play.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::LatiasEx &&
             !after.retreat_used,
         "No-discard-control must keep Latias Active and retain Retreat.");
}

}  // namespace

int main() {
  try {
    test_live_route_promotes_and_attacks();
    test_hold_gate_preserves_latias_and_retreat();
    test_rule_box_lock_blocks_skyliner_route();
    test_live_route_precedes_powerglass_attachment();
    test_no_discard_control_preserves_deterministic_continuation();
    std::cout << "Issue 1325 Latias Celestial Roar tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

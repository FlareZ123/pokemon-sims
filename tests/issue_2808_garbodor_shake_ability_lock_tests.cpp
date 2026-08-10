#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct Issue2808EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool garbodor_locked(const Engine& engine) {
    return engine.garbodor_abilities_locked();
  }
  static bool pokemon_ability_available(const Engine& engine, const Card card) {
    return engine.ability_available_for_pokemon_garbodor(card);
  }
  static bool play_field_blower(Engine& engine) {
    return engine.play_field_blower();
  }
  static bool play_arven(Engine& engine) {
    return engine.play_arven();
  }
  static bool use_fss(Engine& engine) {
    return engine.use_fss();
  }
  static void begin_turn(Engine& engine, const int turn) {
    engine.begin_turn(turn);
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

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

sim::State minimal_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, sim::Card::Crispin};
  return state;
}

void test_registry_contains_both_seats() {
  const auto scenarios = sim::all_scenarios_with_garbodor();
  const auto has = [&scenarios](const std::string& label) {
    return std::any_of(scenarios.begin(), scenarios.end(), [&label](const sim::Scenario& scenario) {
      return scenario.label == label;
    });
  };
  // Boost Shake establishes Garbodor during the opponent's first turn, so both
  // player seats need explicit aggregate rows with different T1 timing:
  // https://api.pokemontcg.io/v2/cards/swsh7-142
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  expect(has("garbodor-shake-ability-lock/go-first"),
         "Garbodor go-first aggregate scenario must be registered.");
  expect(has("garbodor-shake-ability-lock/go-second"),
         "Garbodor go-second aggregate scenario must be registered.");
}

void test_boost_shake_timing_by_seat() {
  std::mt19937_64 rng1{280801};
  const sim::Scenario first{"garbodor-shake-ability-lock/go-first",
                            sim::DciProfile::StrictJit, sim::LockMode::None, true, 5};
  sim::Engine first_engine = make_engine(first, rng1);
  sim::State first_state = minimal_state(1);
  sim::Issue2808EngineTestAccess::set_state(first_engine, first_state);

  // Going first: Regidrago T1 occurs before the opponent can use Boost Shake.
  // The opponent then evolves Garbodor on its first turn, so Garbotoxin begins on
  // Regidrago T2. Garbotoxin suppresses Rule Box and non-Rule-Box Pokemon Abilities:
  // https://api.pokemontcg.io/v2/cards/swsh7-142
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  expect(!sim::Issue2808EngineTestAccess::garbodor_locked(first_engine),
         "Going-first T1 must remain unlocked before the opponent's Boost Shake turn.");
  expect(sim::Issue2808EngineTestAccess::pokemon_ability_available(first_engine, sim::Card::Oricorio),
         "Non-Rule-Box Pokemon Abilities must be available on going-first T1.");

  first_state.turn = 2;
  sim::Issue2808EngineTestAccess::set_state(first_engine, first_state);
  expect(sim::Issue2808EngineTestAccess::garbodor_locked(first_engine),
         "Going-first T2 must be under Garbotoxin.");
  expect(!sim::Issue2808EngineTestAccess::pokemon_ability_available(first_engine, sim::Card::Oricorio),
         "Garbotoxin must suppress non-Rule-Box Pokemon Abilities.");
  expect(!sim::Issue2808EngineTestAccess::pokemon_ability_available(first_engine, sim::Card::CrobatV),
         "Garbotoxin must suppress Pokemon V Abilities.");

  std::mt19937_64 rng2{280802};
  const sim::Scenario second{"garbodor-shake-ability-lock/go-second",
                             sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine second_engine = make_engine(second, rng2);
  sim::Issue2808EngineTestAccess::set_state(second_engine, minimal_state(1));
  expect(sim::Issue2808EngineTestAccess::garbodor_locked(second_engine),
         "Going-second T1 must start under the opponent's established Garbotoxin.");
}

void test_field_blower_unlocks_only_current_turn() {
  std::mt19937_64 rng{280803};
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = minimal_state(1);
  state.hand = {sim::Card::FieldBlower, sim::Card::CrobatV};
  sim::Issue2808EngineTestAccess::set_state(engine, std::move(state));

  // Crobat V's Dark Asset is a live setup connector in this low-hand state. Field
  // Blower may discard the opponent Garbodor's Tool, ending Garbotoxin for this turn:
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  expect(sim::Issue2808EngineTestAccess::play_field_blower(engine),
         "Field Blower must be playable when Garbotoxin blocks a live Dark Asset route.");
  expect(sim::Issue2808EngineTestAccess::pokemon_ability_available(engine, sim::Card::CrobatV),
         "Pokemon Abilities must become available immediately after Field Blower.");
  expect(sim::Issue2808EngineTestAccess::state(engine).path_lock_removed,
         "The current-turn Garbodor Tool-removal marker must be set.");

  sim::Issue2808EngineTestAccess::begin_turn(engine, 2);
  expect(!sim::Issue2808EngineTestAccess::state(engine).path_lock_removed,
         "The opponent re-Tool assumption must clear the temporary unlock next turn.");
  expect(sim::Issue2808EngineTestAccess::garbodor_locked(engine),
         "Garbotoxin must be active again on the next Regidrago turn.");
}

void test_only_actual_item_lock_blocks_field_blower() {
  std::mt19937_64 rng1{280804};
  const sim::Scenario ability_only{"garbodor-shake-ability-lock/go-second",
                                   sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine ability_engine = make_engine(ability_only, rng1);
  sim::State ability_state = minimal_state(1);
  ability_state.hand = {sim::Card::FieldBlower, sim::Card::CrobatV};
  sim::Issue2808EngineTestAccess::set_state(ability_engine, std::move(ability_state));
  expect(sim::Issue2808EngineTestAccess::play_field_blower(ability_engine),
         "Garbotoxin itself must not prohibit Item play.");

  std::mt19937_64 rng2{280805};
  const sim::Scenario item_lock{"garbodor-shake-ability-lock/go-second",
                                sim::DciProfile::StrictJit, sim::LockMode::FullItem, false, 5};
  sim::Engine item_engine = make_engine(item_lock, rng2);
  sim::State item_state = minimal_state(1);
  item_state.hand = {sim::Card::FieldBlower, sim::Card::CrobatV};
  sim::Issue2808EngineTestAccess::set_state(item_engine, std::move(item_state));
  // Item legality is controlled by the actual Item-lock mode, independently from
  // Garbotoxin's Pokemon-Ability suppression:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  expect(!sim::Issue2808EngineTestAccess::play_field_blower(item_engine),
         "A real Item lock must block Field Blower.");
}

void test_arven_searches_field_blower_for_live_unlock() {
  std::mt19937_64 rng{280806};
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = minimal_state(1);
  state.hand = {sim::Card::Arven, sim::Card::CrobatV};
  state.deck = {sim::Card::FieldBlower, sim::Card::ForestSealStone,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  sim::Issue2808EngineTestAccess::set_state(engine, std::move(state));

  // Arven may search one Item and one Pokemon Tool. When Garbotoxin blocks the held
  // Dark Asset connector, Field Blower is a live Item target rather than a generic
  // matchup answer:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  expect(sim::Issue2808EngineTestAccess::play_arven(engine),
         "Arven must take the live Garbotoxin-answer route.");
  expect(sim::Issue2808EngineTestAccess::state(engine).supporter_used,
         "Arven must consume the Supporter action.");
  expect(std::find(sim::Issue2808EngineTestAccess::state(engine).hand.begin(),
                   sim::Issue2808EngineTestAccess::state(engine).hand.end(),
                   sim::Card::FieldBlower) != sim::Issue2808EngineTestAccess::state(engine).hand.end(),
         "Arven must search Field Blower when the unlock advances setup.");
  expect(sim::Issue2808EngineTestAccess::play_field_blower(engine),
         "The searched Field Blower must unlock the Ability route in the same turn.");
}

void test_forest_seal_stone_remains_usable_under_garbotoxin() {
  std::mt19937_64 rng{280807};
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = minimal_state(1);
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.active->tool = sim::Tool::ForestSealStone;
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite,
                sim::Card::MysteriousTreasure};
  sim::Issue2808EngineTestAccess::set_state(engine, std::move(state));

  // The repository's existing ruling treats Star Alchemy as the Tool's Ability that
  // the attached Pokemon V may use. Garbotoxin suppresses Pokemon Abilities, so this
  // Tool Ability remains available just as it does through Path to the Peak:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  expect(sim::Issue2808EngineTestAccess::garbodor_locked(engine),
         "The exact state must actually be under Garbotoxin.");
  expect(sim::Issue2808EngineTestAccess::use_fss(engine),
         "Star Alchemy must remain usable while Garbotoxin suppresses Pokemon Abilities.");
}
}  // namespace

int main() {
  test_registry_contains_both_seats();
  test_boost_shake_timing_by_seat();
  test_field_blower_unlocks_only_current_turn();
  test_only_actual_item_lock_blocks_field_blower();
  test_arven_searches_field_blower_for_live_unlock();
  test_forest_seal_stone_remains_usable_under_garbotoxin();
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool ability_available(const Engine& engine, const Card card) {
    return engine.ability_available_for_pokemon(card);
  }
  static bool garbodor_locked(const Engine& engine) {
    return engine.garbodor_abilities_locked();
  }
  static bool play_garbodor_field_blower(Engine& engine) {
    return engine.play_garbodor_field_blower();
  }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario garbodor_scenario(const bool going_first) {
  return sim::Scenario{
      std::string("strict-jit-garbodor-shake-ability-lock/") +
          (going_first ? "go-first" : "go-second"),
      sim::DciProfile::StrictJit, sim::LockMode::None, going_first, 5};
}

void test_boost_shake_turn_timing() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 first_rng{2808};
  std::mt19937_64 second_rng{2809};
  sim::Engine first(garbodor_scenario(true), recipe, first_rng);
  sim::Engine second(garbodor_scenario(false), recipe, second_rng);

  sim::State first_t1;
  first_t1.turn = 1;
  first_t1.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  sim::EngineTestAccess::set_state(first, first_t1);
  expect(!sim::EngineTestAccess::garbodor_locked(first),
         "Going-first T1 should occur before the opponent's Boost Shake turn.");
  expect(sim::EngineTestAccess::ability_available(first, sim::Card::Oricorio),
         "Going-first T1 incorrectly suppresses Pokemon Abilities.");

  sim::State first_t2 = first_t1;
  first_t2.turn = 2;
  sim::EngineTestAccess::set_state(first, first_t2);
  expect(sim::EngineTestAccess::garbodor_locked(first),
         "Going-first T2 should be under established Garbotoxin.");
  expect(!sim::EngineTestAccess::ability_available(first, sim::Card::Oricorio),
         "Garbotoxin must suppress non-Rule-Box Pokemon Abilities too.");

  sim::State second_t1;
  second_t1.turn = 1;
  second_t1.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  sim::EngineTestAccess::set_state(second, second_t1);
  expect(sim::EngineTestAccess::garbodor_locked(second),
         "Going-second T1 should begin after the opponent's Boost Shake turn.");
  expect(!sim::EngineTestAccess::ability_available(second, sim::Card::TapuLeleGX),
         "Going-second T1 incorrectly leaves Wonder Tag available.");

  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Core turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
}

void test_field_blower_unlocks_only_current_turn() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{2810};
  sim::Engine engine(garbodor_scenario(false), recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 0};
  state.hand = {sim::Card::FieldBlower};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, state);

  expect(!sim::EngineTestAccess::ability_available(engine, sim::Card::RegidragoVstar),
         "Legacy Star should begin suppressed by Garbotoxin.");
  expect(sim::EngineTestAccess::play_garbodor_field_blower(engine),
         "Field Blower should spend Garbodor's attached Tool when a live Ability route exists.");
  expect(sim::EngineTestAccess::ability_available(engine, sim::Card::RegidragoVstar),
         "Field Blower did not restore Pokemon Abilities for the current turn.");
  expect(sim::EngineTestAccess::ability_available(engine, sim::Card::Oricorio),
         "Field Blower should restore non-Rule-Box Pokemon Abilities too.");
  expect(std::count(sim::EngineTestAccess::state(engine).discard.begin(),
                    sim::EngineTestAccess::state(engine).discard.end(),
                    sim::Card::FieldBlower) == 1,
         "Field Blower was not consumed by the Garbodor unlock.");

  sim::State next_turn = sim::EngineTestAccess::state(engine);
  next_turn.turn = 2;
  sim::EngineTestAccess::set_state(engine, std::move(next_turn));
  expect(sim::EngineTestAccess::garbodor_locked(engine),
         "Maximum-pressure Garbodor should be re-Tooled before the next player turn.");
  expect(!sim::EngineTestAccess::ability_available(engine, sim::Card::Oricorio),
         "The Field Blower unlock incorrectly persisted into the next turn.");

  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Core Tool, Item, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
}

void test_forest_seal_stone_remains_available() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{2811};
  sim::Engine engine(garbodor_scenario(false), recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::ForestSealStone};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::garbodor_locked(engine),
         "Control state should begin under Garbotoxin.");
  expect(sim::EngineTestAccess::use_fss(engine),
         "Forest Seal Stone's Tool-granted Star Alchemy was incorrectly suppressed.");

  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Existing repository FSS interpretation is intentionally preserved by #2808.
}

void test_arven_fetches_and_immediately_uses_field_blower() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{2812};
  sim::Engine engine(garbodor_scenario(false), recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 0};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::FieldBlower, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_arven(engine),
         "Arven should fetch Field Blower when Garbotoxin blocks a live Legacy Star route.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Arven did not consume the Supporter action.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::FieldBlower) == 1,
         "Arven's searched Field Blower was not played after the Supporter resolved.");
  expect(sim::EngineTestAccess::ability_available(engine, sim::Card::RegidragoVstar),
         "Arven into Field Blower did not unlock the current-turn Legacy Star route.");

  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Core Supporter then Item sequencing: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
}

void test_scenario_rows_and_aliases() {
  const auto scenarios = sim::all_scenarios();
  expect(scenarios.size() == 16U,
         "Garbodor enhancement should append exactly two rows to the prior 14 scenarios.");
  expect(scenarios[14].label == "strict-jit-garbodor-shake-ability-lock/go-first",
         "Going-first Garbodor row did not preserve old scenario ordering.");
  expect(scenarios[15].label == "strict-jit-garbodor-shake-ability-lock/go-second",
         "Going-second Garbodor row did not preserve old scenario ordering.");
  expect(sim::scenario_by_label("garbodor-shake-ability-lock/go-first").has_value(),
         "Concise going-first Garbodor scenario alias is missing.");
  expect(sim::scenario_by_label("garbodor-shake-ability-lock/go-second").has_value(),
         "Concise going-second Garbodor scenario alias is missing.");
}

}  // namespace

int main() {
  try {
    test_boost_shake_turn_timing();
    test_field_blower_unlocks_only_current_turn();
    test_forest_seal_stone_remains_available();
    test_arven_fetches_and_immediately_uses_field_blower();
    test_scenario_rows_and_aliases();
    std::cout << "Issue 2808 Garbodor Boost Shake ability-lock tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

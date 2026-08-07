#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {
  static void set_turn(Engine& engine, const int turn) { engine.state_.turn = turn; }
  static bool item_locked(const Engine& engine) { return engine.item_locked(); }
  static bool rule_box_locked(const Engine& engine) {
    return engine.rule_box_abilities_locked();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(const sim::Scenario& scenario,
                        const sim::DeckRecipe& recipe,
                        std::mt19937_64& rng) {
  return sim::Engine(scenario, recipe, rng);
}

void retired_labels_are_absent() {
  // Current-paper aggregate and trace registration intentionally excludes every
  // full-turn-one Item-lock label. The starting player skips the first-turn attack,
  // and Forest of Giant Plants is banned in Expanded:
  // https://assets.pokemon.com/assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf
  // https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  expect(sim::all_scenarios().size() == 14U,
         "Aggregate registry must contain exactly 14 current scenarios");
  expect(!sim::scenario_by_label("strict-jit-full-item-lock/go-first"),
         "Retired go-first full Item-lock label was re-registered");
  expect(!sim::scenario_by_label("strict-jit-full-item-lock/go-second"),
         "Retired go-second full Item-lock label was re-registered");
}

void turn_two_item_lock_has_t1_window() {
  const auto scenario = sim::scenario_by_label("strict-jit-turn2-item-lock/go-first");
  expect(scenario.has_value(), "Turn-two Item-lock scenario is missing");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{224701};
  sim::Engine engine = make_engine(*scenario, recipe, rng);
  sim::EngineTestAccess::set_turn(engine, 1);
  expect(!sim::EngineTestAccess::item_locked(engine),
         "TurnTwoItem incorrectly suppresses Items on T1");
  sim::EngineTestAccess::set_turn(engine, 2);
  expect(sim::EngineTestAccess::item_locked(engine),
         "TurnTwoItem failed to suppress Items on T2");
}

void combined_lock_uses_turn_two_item_timing() {
  const auto scenario = sim::scenario_by_label("strict-jit-combined-lock/go-first");
  expect(scenario.has_value(), "Combined-lock scenario is missing");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{224702};
  sim::Engine engine = make_engine(*scenario, recipe, rng);

  // Combined lock keeps Path-style Rule Box Ability suppression from the start,
  // while its Item component follows the same T2 timing as TurnTwoItem:
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Current turn procedure: https://assets.pokemon.com/assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf
  // Repository contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  // Confirmed cleanup: https://github.com/FlareZ123/pokemon-sims/issues/2247
  sim::EngineTestAccess::set_turn(engine, 1);
  expect(!sim::EngineTestAccess::item_locked(engine),
         "Combined lock incorrectly suppresses Items on T1");
  expect(sim::EngineTestAccess::rule_box_locked(engine),
         "Combined lock lost its T1 Rule Box Ability suppression");
  sim::EngineTestAccess::set_turn(engine, 2);
  expect(sim::EngineTestAccess::item_locked(engine),
         "Combined lock failed to suppress Items on T2");
  expect(sim::EngineTestAccess::rule_box_locked(engine),
         "Combined lock lost Rule Box Ability suppression on T2");
}

void full_item_remains_synthetic_only() {
  const sim::Scenario scenario{"issue-2247-synthetic-full-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, true, 2};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{224703};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_turn(engine, 1);
  // Retaining this internal mode protects historical exact-state regressions without
  // restoring it to current-paper scenario registration:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  expect(sim::EngineTestAccess::item_locked(engine),
         "Synthetic FullItem fixture must still lock Items on T1");
}

}  // namespace

int main() {
  try {
    retired_labels_are_absent();
    turn_two_item_lock_has_t1_window();
    combined_lock_uses_turn_two_item_timing();
    full_item_remains_synthetic_only();
    std::cout << "Issue 2247 lock timing tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "issue-2247 lock timing test failure: " << error.what() << '\n';
    return 1;
  }
}

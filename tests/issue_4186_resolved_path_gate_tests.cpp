#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_turn(Engine& engine, const int turn) {
    engine.state_.turn = turn;
  }

  static void set_path_lock_removed(Engine& engine, const bool removed) {
    engine.state_.path_lock_removed = removed;
  }

  static bool item_locked_now(const Engine& engine) {
    return engine.item_locked();
  }

  static bool item_locked_on_turn(const Engine& engine, const int turn) {
    return engine.item_locked_on_turn(turn);
  }

  static bool tapu_ability_available(const Engine& engine) {
    return engine.ability_available_for_pokemon(Card::TapuLeleGX);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(const sim::LockMode locks, std::mt19937_64& rng) {
  const sim::Scenario scenario{
      "issue-4186/resolved-path", sim::DciProfile::StrictJit, locks, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

void test_resolved_path_uses_live_ability_legality() {
  // Path disables Rule Box Pokemon Abilities only while its continuous Stadium
  // effect remains active. Field Blower can discard the Stadium, so the #1552
  // T1 Vessel gate must consult the live path_lock_removed state before deciding
  // whether T2 Tapu Lele-GX -> Wonder Tag is legal.
  // Path to the Peak: https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/swsh6.json
  // Field Blower / Tapu Lele-GX: https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/sm2.json
  // Stadium procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#b-04-stadiums
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4186
  std::mt19937_64 rng(4186);
  sim::Engine engine = make_engine(sim::LockMode::FullRuleBoxAbility, rng);
  sim::EngineTestAccess::set_turn(engine, 1);

  expect(!sim::EngineTestAccess::item_locked_now(engine),
         "Rule Box Ability lock unexpectedly blocked T1 Items.");
  expect(!sim::EngineTestAccess::item_locked_on_turn(engine, 2),
         "Rule Box Ability lock unexpectedly became a T2 Item lock.");
  expect(!sim::EngineTestAccess::tapu_ability_available(engine),
         "Unresolved Path unexpectedly allowed Tapu Lele-GX Wonder Tag.");

  sim::EngineTestAccess::set_path_lock_removed(engine, true);
  expect(sim::EngineTestAccess::tapu_ability_available(engine),
         "Resolved Path should restore Tapu Lele-GX Wonder Tag legality.");
}

void test_full_combined_remains_blocked_by_future_item_lock() {
  // Earthen Vessel is an Item and must be played before a scheduled Item lock
  // when the continuation still depends on Item access. FullCombined retains
  // that T2 Item lock even after its Path-style Ability lock is removed.
  // Earthen Vessel: https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/sv4.json
  // Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#b-01-items
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/4186
  std::mt19937_64 rng(4187);
  sim::Engine engine = make_engine(sim::LockMode::FullCombined, rng);
  sim::EngineTestAccess::set_turn(engine, 1);
  sim::EngineTestAccess::set_path_lock_removed(engine, true);

  expect(!sim::EngineTestAccess::item_locked_now(engine),
         "FullCombined should not retroactively Item-lock turn one.");
  expect(sim::EngineTestAccess::item_locked_on_turn(engine, 2),
         "FullCombined must retain its scheduled turn-two Item lock.");
  expect(sim::EngineTestAccess::tapu_ability_available(engine),
         "Removing Path should restore Tapu Lele-GX Ability legality even when a separate Item lock remains scheduled.");
}

}  // namespace

int main() {
  try {
    test_resolved_path_uses_live_ability_legality();
    test_full_combined_remains_blocked_by_future_item_lock();
    std::cout << "Issue 4186 resolved-Path gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

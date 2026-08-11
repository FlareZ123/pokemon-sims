#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool play_field_blower(Engine& engine) {
    return engine.play_field_blower();
  }
  static bool attach_powerglass(Engine& engine) {
    return engine.attach_powerglass();
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

sim::Scenario scenario() {
  return sim::Scenario{"issue-2807", sim::DciProfile::StrictJit,
                       sim::LockMode::None, false, 5};
}

sim::State spent_fss_state() {
  sim::State state;
  state.turn = 2;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 0, 1, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::FieldBlower, sim::Card::Powerglass};
  state.discard = {sim::Card::Grass};
  return state;
}

void test_spent_fss_can_be_cleared_for_powerglass() {
  std::mt19937_64 rng{2807};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario owned_scenario = scenario();
  sim::Engine engine(owned_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, spent_fss_state());

  // Field Blower may choose the player's own Pokémon Tool and discard it. Once a
  // VSTAR Power has been used, Forest Seal Stone cannot provide Star Alchemy again;
  // clearing that spent Tool slot lets another Tool be attached later this turn.
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Core Tool/Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2807
  expect(sim::EngineTestAccess::play_field_blower(engine),
         "Field Blower did not clear a spent Forest Seal Stone for a live Tool route.");
  const sim::State& after_blower = sim::EngineTestAccess::state(engine);
  expect(after_blower.active && after_blower.active->tool == sim::Tool::None,
         "Field Blower left the spent Forest Seal Stone in the Tool slot.");
  expect(std::count(after_blower.discard.begin(), after_blower.discard.end(),
                    sim::Card::ForestSealStone) == 1,
         "Discarded Forest Seal Stone was not recorded in the discard pile.");
  expect(std::count(after_blower.discard.begin(), after_blower.discard.end(),
                    sim::Card::FieldBlower) == 1,
         "Played Field Blower was not recorded in the discard pile.");

  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "Powerglass could not use the Tool slot cleared by Field Blower.");
  expect(sim::EngineTestAccess::state(engine).active->tool == sim::Tool::Powerglass,
         "Powerglass was not attached after the spent Stone was cleared.");
}

void test_live_fss_is_preserved_without_a_path_target() {
  std::mt19937_64 rng{2808};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario owned_scenario = scenario();
  sim::Engine engine(owned_scenario, recipe, rng);
  sim::State state = spent_fss_state();
  state.vstar_power_used = false;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Forest Seal Stone remains live before the one-per-game VSTAR Power is spent,
  // so the policy must not burn Field Blower merely to replace a still-useful Tool.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // VSTAR Power procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // DCI/UDP policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  expect(!sim::EngineTestAccess::play_field_blower(engine),
         "Field Blower discarded a live Forest Seal Stone without another legal target.");
  expect(sim::EngineTestAccess::state(engine).active->tool ==
             sim::Tool::ForestSealStone,
         "Live Forest Seal Stone was incorrectly removed.");
}
}  // namespace

int main() {
  try {
    test_spent_fss_can_be_cleared_for_powerglass();
    test_live_fss_is_preserved_without_a_path_target();
    std::cout << "Issue 2807 Field Blower spent FSS tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

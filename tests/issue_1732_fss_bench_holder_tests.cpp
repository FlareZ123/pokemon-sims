#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool attach(Engine& engine) {
    return engine.attach_fss_for_crobat_compression();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(sim::Scenario& scenario, sim::DeckRecipe& recipe,
                        std::mt19937_64& rng) {
  return sim::Engine(scenario, recipe, rng);
}

void install_common_state(sim::Engine& engine) {
  auto& state = sim::EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone, sim::Card::CrobatV,
                sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire};
}

void test_benched_holder_preserves_active_tool_slot() {
  sim::Scenario scenario{"issue-1732-bench-first", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1732};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_common_state(engine);
  auto& state = sim::EngineTestAccess::state(engine);
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None}};

  // Forest Seal Stone may attach to either Pokémon V. Using the Benched Regidrago
  // preserves the Active Tool slot for a later Powerglass while retaining the same
  // hand compression and Star Alchemy access:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Official Pokémon Tool procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1732
  expect(sim::EngineTestAccess::attach(engine),
         "The legal Forest Seal Stone compression was rejected.");
  expect(state.active->tool == sim::Tool::None,
         "Forest Seal Stone consumed the Active Tool slot despite an open Benched holder.");
  expect(state.bench.front().tool == sim::Tool::ForestSealStone,
         "Forest Seal Stone did not use the open Benched Regidrago holder.");
  expect(sim::count_of(state.hand, sim::Card::ForestSealStone) == 0,
         "Forest Seal Stone remained in hand after attachment.");
}

void test_active_holder_remains_legal_fallback() {
  sim::Scenario scenario{"issue-1732-active-fallback", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1733};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_common_state(engine);
  auto& state = sim::EngineTestAccess::state(engine);
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None}};

  // The Active Regidrago remains the legal fallback when no open Benched Pokémon V
  // holder exists:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Official Pokémon Tool procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1732
  expect(sim::EngineTestAccess::attach(engine),
         "The Active-only Forest Seal Stone fallback was rejected.");
  expect(state.active->tool == sim::Tool::ForestSealStone,
         "Forest Seal Stone did not use the only legal Active holder.");
}

void test_occupied_benched_holder_falls_back_to_active() {
  sim::Scenario scenario{"issue-1732-occupied-bench", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1734};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_common_state(engine);
  auto& state = sim::EngineTestAccess::state(engine);
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 0, 0, 0,
                              sim::Tool::Powerglass}};

  // A Pokémon with a Tool already attached is not an open Forest Seal Stone holder,
  // so the selector must retain its Active fallback:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Official one-Tool-per-Pokémon procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1732
  expect(sim::EngineTestAccess::attach(engine),
         "The occupied-Bench fallback route was rejected.");
  expect(state.bench.front().tool == sim::Tool::Powerglass,
         "The selector replaced the Benched Pokémon's existing Tool.");
  expect(state.active->tool == sim::Tool::ForestSealStone,
         "The selector did not fall back to the open Active holder.");
}
}  // namespace

int main() {
  test_benched_holder_preserves_active_tool_slot();
  test_active_holder_remains_legal_fallback();
  test_occupied_benched_holder_falls_back_to_active();
}

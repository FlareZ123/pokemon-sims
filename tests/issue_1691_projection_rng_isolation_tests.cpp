#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool issue_1235_completion(const Engine& engine) {
    return engine.issue_1235_t2_treasure_tapu_crispin_completion_available();
  }
  static bool issue_1209_completion(const Engine& engine) {
    return engine.issue_1209_t2_treasure_tapu_crispin_completion_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State issue_1235_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0},
                 sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::DialgaGX,
                sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::Gladion};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::Arven};
  return state;
}

sim::State issue_1209_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dragapult,
                sim::Card::RegidragoVstar, sim::Card::Arven};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::MegaDragonite};
  state.manual_energy_used = true;
  return state;
}

void verify_projection_isolation(const sim::Scenario& scenario,
                                 sim::State state,
                                 const std::uint64_t seed,
                                 const bool issue_1235) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  std::mt19937_64 expected_rng = rng;
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Both completion predicates silently execute Mysterious Treasure, Wonder Tag,
  // and Crispin searches. Their projected shuffles must leave the live trial stream
  // at the exact pre-projection position when the predicate returns:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Official search and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // C++ reference-member copy semantics: https://eel.is/c++draft/class.copy.ctor#15
  // Repository fixed-seed sampling contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#sampling-and-comparison-method
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1691
  const bool available = issue_1235
      ? sim::EngineTestAccess::issue_1235_completion(engine)
      : sim::EngineTestAccess::issue_1209_completion(engine);
  expect(available, "The source-bounded completion route must remain available.");
  expect(trace.lines.empty(),
         "The silent completion projection leaked speculative trace output.");
  expect(rng() == expected_rng(),
         "The silent completion projection advanced the live trial RNG.");
}

void test_issue_1235_projection_preserves_rng() {
  verify_projection_isolation(
      sim::Scenario{"issue-1691/1235", sim::DciProfile::StrictJit,
                    sim::LockMode::None, false, 5},
      issue_1235_state(), 123502, true);
}

void test_issue_1209_projection_preserves_rng() {
  verify_projection_isolation(
      sim::Scenario{"issue-1691/1209", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 5},
      issue_1209_state(), 120902, false);
}

}  // namespace

int main() {
  try {
    test_issue_1235_projection_preserves_rng();
    test_issue_1209_projection_preserves_rng();
    std::cout << "Issue 1691 projection RNG isolation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

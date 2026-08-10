#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <type_traits>

namespace sim {
struct EngineTestAccess {
  static constexpr bool owns_scenario =
      !std::is_reference_v<decltype(Engine::scenario_)>;

  static DciProfile scenario_dci(const Engine& engine) {
    return engine.scenario_.dci;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_engine_owns_temporary_scenario() {
  static_assert(
      sim::EngineTestAccess::owns_scenario,
      "Engine must own Scenario state instead of retaining a caller reference.");

  std::mt19937_64 rng{2815};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(
      sim::Scenario{"issue-2815", sim::DciProfile::MatchupFlexJit,
                    sim::LockMode::None, false, 3},
      recipe, rng);

  // A temporary Scenario is destroyed at the end of the constructor expression.
  // Engine must retain an owned copy for every later policy read.
  // C++ temporary lifetime: https://eel.is/c++draft/class.temporary
  // Confirmed lifetime bug: https://github.com/FlareZ123/pokemon-sims/issues/2815
  expect(sim::EngineTestAccess::scenario_dci(engine) ==
             sim::DciProfile::MatchupFlexJit,
         "Engine lost the copied temporary Scenario state.");
}
}  // namespace

int main() {
  try {
    test_engine_owns_temporary_scenario();
    std::cout << "Issue 2815 Engine Scenario lifetime tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

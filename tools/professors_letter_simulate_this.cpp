#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {

DeckRecipe professors_letter_demo_recipe() {
  DeckRecipe recipe = baseline_recipe();
  const auto vessel = std::find_if(
      recipe.begin(), recipe.end(),
      [](const auto& entry) { return entry.first == Card::EarthenVessel; });
  if (vessel == recipe.end() || vessel->second != 2) {
    throw std::logic_error(
        "regidrago-shell no longer has exactly two Earthen Vessel");
  }

  // Preserve the recipe entry position so a same-seed comparison applies the
  // exact same shuffle permutation to every unchanged card. The two physical
  // Earthen Vessel slots alone become Professor's Letter slots.
  // Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2509
  vessel->first = Card::ProfessorsLetter;

  NamedDeck validation{"professors-letter-temporary-swap", recipe};
  std::string error;
  if (!validate_recipe(validation, &error)) throw std::logic_error(error);
  return recipe;
}

int simulate_this(const std::string& scenario_label, const std::uint64_t seed) {
  const auto scenario = scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("unknown scenario: " + scenario_label);

  std::mt19937_64 rng(seed);
  TraceLog trace{true, {}};
  Engine engine(*scenario, professors_letter_demo_recipe(), rng, &trace);
  const TrialOutcome outcome = engine.run();

  std::cout << "Professor's Letter demo | scenario=" << scenario_label
            << " | seed=" << seed << '\n';
  for (const std::string& line : trace.lines) std::cout << line << '\n';
  std::cout << "RESULT | first_ready_turn=" << outcome.first_ready_turn
            << " | ready_by_2=" << (outcome.ready_by_2 ? 1 : 0)
            << " | ready_by_3=" << (outcome.ready_by_3 ? 1 : 0)
            << " | ready_by_4=" << (outcome.ready_by_4 ? 1 : 0) << '\n';
  return 0;
}

}  // namespace sim

int main(int argc, char** argv) {
  bool simulate = false;
  std::string scenario;
  std::uint64_t seed = 0;
  bool seed_supplied = false;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--simulate-this") {
      simulate = true;
    } else if (arg == "--scenario" && i + 1 < argc) {
      scenario = argv[++i];
    } else if (arg == "--seed" && i + 1 < argc) {
      seed = std::stoull(argv[++i]);
      seed_supplied = true;
    } else {
      std::cerr << "unknown argument: " << arg << '\n';
      return 2;
    }
  }

  if (!simulate || scenario.empty() || !seed_supplied) {
    std::cerr << "usage: professors_letter_simulate_this --simulate-this --scenario LABEL --seed N\n";
    return 2;
  }
  return sim::simulate_this(scenario, seed);
}

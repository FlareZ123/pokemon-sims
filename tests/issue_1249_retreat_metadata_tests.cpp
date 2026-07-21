#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool use_exploding_energy(Engine& engine) {
    return engine.use_exploding_energy_for_setup();
  }
};

}  // namespace sim

namespace {

void test_exact_printed_retreat_costs() {
  // Exact printed Retreat Cost records:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Mawile-GX: https://api.pokemontcg.io/v2/cards/sm11-141
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Regression scope: https://github.com/FlareZ123/pokemon-sims/issues/1301
  if (sim::retreat_cost(sim::Card::RegidragoV) != 3 ||
      sim::retreat_cost(sim::Card::RegidragoVstar) != 3 ||
      sim::retreat_cost(sim::Card::TapuLeleGX) != 1 ||
      sim::retreat_cost(sim::Card::Pineco) != 2 ||
      sim::retreat_cost(sim::Card::DialgaGX) != 3 ||
      sim::retreat_cost(sim::Card::LatiasEx) != 2 ||
      sim::retreat_cost(sim::Card::MawileGX) != 1 ||
      sim::retreat_cost(sim::Card::Oricorio) != 1) {
    throw std::runtime_error(
        "Modeled printed Retreat Cost metadata diverged from the exact card records.");
  }
}

void test_exploding_energy_does_not_promote_mawile_for_free() {
  const sim::Scenario scenario{"issue-1301/mawile-retreat",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe = sim::pineco_recipe();
  std::mt19937_64 rng{1301};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::MawileGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::ForretressEx, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1, sim::Tool::None},
  };
  state.deck = {sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Exploding Energy may attach the two Grass to Regidrago, but Mawile-GX's
  // printed one-Energy Retreat Cost prevents promotion when no third Energy is
  // available to pay that cost:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1301
  if (!sim::EngineTestAccess::use_exploding_energy(engine)) {
    throw std::runtime_error("Exploding Energy did not resolve in the Mawile fixture.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  const auto regi = std::find_if(
      after.bench.begin(), after.bench.end(), [](const sim::Pokemon& pokemon) {
        return pokemon.card == sim::Card::RegidragoV;
      });
  if (!after.active || after.active->card != sim::Card::MawileGX ||
      after.retreat_used || regi == after.bench.end() || regi->grass != 2 ||
      regi->fire != 1) {
    throw std::runtime_error(
        "Exploding Energy promoted through Mawile-GX without paying its Retreat Cost.");
  }
}

}  // namespace

int main() {
  try {
    test_exact_printed_retreat_costs();
    test_exploding_energy_does_not_promote_mawile_for_free();
    std::cout << "Issue 1249/1301 retreat metadata tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

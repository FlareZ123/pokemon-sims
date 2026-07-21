#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

void test_exact_printed_retreat_costs() {
  // Regidrago V swsh12-135 has three printed Colorless Retreat symbols:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // Latias ex remains covered by issue #1248 because its printed metadata and
  // effective Skyliner cost require a separate behavioral correction:
  // https://github.com/FlareZ123/pokemon-sims/issues/1248
  if (sim::retreat_cost(sim::Card::RegidragoV) != 3 ||
      sim::retreat_cost(sim::Card::RegidragoVstar) != 3 ||
      sim::retreat_cost(sim::Card::TapuLeleGX) != 1 ||
      sim::retreat_cost(sim::Card::Pineco) != 2 ||
      sim::retreat_cost(sim::Card::DialgaGX) != 3 ||
      sim::retreat_cost(sim::Card::Oricorio) != 1) {
    throw std::runtime_error("Modeled printed Retreat Cost metadata diverged from the exact card records.");
  }
}

}  // namespace

int main() {
  try {
    test_exact_printed_retreat_costs();
    std::cout << "Issue 1249 retreat metadata tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

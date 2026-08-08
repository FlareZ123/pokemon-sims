#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static bool alive() { return true; }
};
}  // namespace sim

int main() {
  // Temporary CI-only high-volume audit for issue #2368. CTest runs from the
  // build directory, so `..` is the checked-out source root in both Release and
  // sanitizer jobs. Compile the scanner separately at O2 so 1.2M paired games do
  // not inherit the unified regression runner's intentional O0 setting.
  // Exact DDE/rules basis:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
  if (!sim::EngineTestAccess::alive()) return 2;
  const int compile_status = std::system(
      "g++ -std=c++20 -O2 -I.. ../scripts/issue2368_paired_readiness_scan.cpp "
      "-o issue2368_paired_readiness_scan");
  if (compile_status != 0) {
    std::cerr << "Issue-2368 paired scanner failed to compile\n";
    return 3;
  }
  const int scan_status = std::system("./issue2368_paired_readiness_scan");
  if (scan_status != 0) {
    std::cerr << "Issue-2368 paired scanner found an exception or failed\n";
    return 4;
  }
  std::cout << "Issue-2368 optimized paired readiness scan completed\n";
  return 0;
}

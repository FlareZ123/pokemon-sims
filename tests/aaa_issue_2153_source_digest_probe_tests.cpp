#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstdlib>

namespace sim {
// Unified test registration contract: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py
// Temporary source-bound evidence probe for issue #2153: https://github.com/FlareZ123/pokemon-sims/issues/2153
struct EngineTestAccess {};
}  // namespace sim

int main() {
  const int status = std::system(
      "python3 -c \"import sys; from pathlib import Path; root=Path('..').resolve(); "
      "sys.path.insert(0,str(root)); from scripts.baseline_provenance import "
      "simulator_policy_source_digest; print('ISSUE2153_DIGEST=' + "
      "simulator_policy_source_digest(root))\"");
  if (status != 0) return status;
  return 1;
}

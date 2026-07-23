#include <cstdlib>
#include <stdexcept>

int main() {
  // Temporary source-bound provenance probe for the required baseline refresh:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/baseline_provenance.py
  if (std::getenv("GITHUB_WORKSPACE") == nullptr) return 0;
  const int status = std::system(
      "python3 -c \"from pathlib import Path; import sys; root=Path(r'$GITHUB_WORKSPACE'); "
      "sys.path.insert(0,str(root)); from scripts.baseline_provenance import "
      "simulator_policy_source_digest; print('ISSUE1392_POLICY_DIGEST=' + "
      "simulator_policy_source_digest(root))\"");
  if (status != 0) throw std::runtime_error("Policy digest probe failed.");
}

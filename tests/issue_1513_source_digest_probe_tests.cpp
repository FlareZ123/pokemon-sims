#include <cstdlib>

int main() {
  return std::system(
      "python3 -c \"from pathlib import Path; import sys; "
      "root=Path('..').resolve(); sys.path.insert(0,str(root)); "
      "from scripts.baseline_provenance import simulator_policy_source_digest; "
      "print('ISSUE1513_SOURCE_DIGEST='+simulator_policy_source_digest(root))\"");
}

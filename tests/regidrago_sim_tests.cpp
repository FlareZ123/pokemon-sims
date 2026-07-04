#include <cassert>
#include <numeric>
#include <vector>

// Lightweight invariants are deliberately dependency-free so that a fresh checkout can run ctest.
int main() {
  const std::vector<int> counts = {4, 3, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 4, 3, 2, 2, 2, 1, 1, 1, 1,
                                   1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 3};
  assert(std::accumulate(counts.begin(), counts.end(), 0) == 60);
  assert(4 + 3 + 2 + 2 + 1 + 1 + 2 + 1 + 1 + 1 + 1 == 19);
  assert(std::accumulate(counts.begin() + 11, counts.begin() + 34, 0) == 32);
  assert(6 + 3 == 9);
  return 0;
}

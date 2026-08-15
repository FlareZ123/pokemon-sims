#pragma once

#include <algorithm>

namespace sim {

// Central arithmetic for K0/K1 copy reasoning. Keep hidden-information policy in
// Engine; this class only combines counts already legal for the caller to know.
// Policy source: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
struct KnowledgeCopyPolicy {
  [[nodiscard]] static constexpr bool has_any(const int count) {
    return count > 0;
  }

  [[nodiscard]] static constexpr int combined(const int first_count,
                                              const int second_count) {
    return first_count + second_count;
  }

  [[nodiscard]] static int unresolved(const int total_copies,
                                      const int known_copies) {
    return std::max(0, total_copies - known_copies);
  }
};

}  // namespace sim

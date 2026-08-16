#ifndef SIM_TRACE_SETUP_RECIPE_POLICY_INCLUDED
#define SIM_TRACE_SETUP_RECIPE_POLICY_INCLUDED

  class SetupRecipePolicy final {
   public:
    [[nodiscard]] static constexpr bool contains_count(const int copies) {
      // Keep setup recipe-presence classification behind one seam rather than
      // duplicating raw copy-count comparisons at deck/scenario label callers.
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#setup-lifecycle-cleanup
      return copies > 0;
    }

    [[nodiscard]] static constexpr bool has_exact_count(
        const int copies, const int expected) {
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#setup-lifecycle-cleanup
      return copies == expected;
    }
  };

#endif  // SIM_TRACE_SETUP_RECIPE_POLICY_INCLUDED

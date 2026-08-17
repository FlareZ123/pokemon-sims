#ifndef SIM_TRACE_SETUP_POLICIES_INCLUDED
#define SIM_TRACE_SETUP_POLICIES_INCLUDED

  // Pure setup metadata policy lives separately from physical setup state changes.
  // Setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#setup-lifecycle-cleanup
  struct SetupRecipePolicy final {
    [[nodiscard]] static constexpr bool contains_count(const int copies) {
      return copies > 0;
    }

    [[nodiscard]] static constexpr bool has_exact_count(
        const int copies, const int expected) {
      return copies == expected;
    }
  };

  struct SetupLifecycleConfig final {
    inline static constexpr int kOpeningHandSize = 7;
    inline static constexpr int kStartingPrizeCount = 6;
    inline static constexpr const char* kUnknownLabel = "unknown";

    [[nodiscard]] static const char* dci_label(const DciProfile profile) {
      // DCI/JIT policy vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      switch (profile) {
        case DciProfile::StrictJit: return "strict-JIT";
        case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
        case DciProfile::NoDiscardControl: return "no-discard-control";
      }
      return kUnknownLabel;
    }

    [[nodiscard]] static const char* lock_label(const LockMode mode) {
      // Scenario lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
      switch (mode) {
        case LockMode::None: return "none";
        case LockMode::TurnTwoItem: return "turn-two Item lock";
        case LockMode::FullItem: return "full Item lock";
        case LockMode::FullRuleBoxAbility: return "full Rule Box Ability lock";
        case LockMode::FullCombined: return "full Item + Rule Box Ability lock";
        case LockMode::FullSupporter: return "full Supporter lock";
      }
      return kUnknownLabel;
    }
  };

#endif  // SIM_TRACE_SETUP_POLICIES_INCLUDED

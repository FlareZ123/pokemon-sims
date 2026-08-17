#ifndef SIM_TRACE_SETUP_LIFECYCLE_CONFIG_INCLUDED
#define SIM_TRACE_SETUP_LIFECYCLE_CONFIG_INCLUDED

  class SetupLifecycleConfig final {
   public:
    // Starting-hand and Prize counts follow the advanced setup procedure:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
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

#endif  // SIM_TRACE_SETUP_LIFECYCLE_CONFIG_INCLUDED

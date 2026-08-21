#ifndef SIM_TRACE_LOCK_MODE_POLICY_INCLUDED
#define SIM_TRACE_LOCK_MODE_POLICY_INCLUDED

// Pure scenario-lock classification lives here so legacy Engine fragments do not
// duplicate which synthetic lock modes affect Supporters, Items, or Rule Box
// Abilities. State-dependent removal and turn actions remain Engine strategy.
// Repository lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
// Scenario policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
// Path to the Peak exact effect: https://api.pokemontcg.io/v2/cards/swsh6-148
struct LockModePolicy final {
  [[nodiscard]] static constexpr bool uses_full_supporter_lock(
      const LockMode locks) {
    return locks == LockMode::FullSupporter;
  }

  [[nodiscard]] static constexpr bool uses_turn_two_item_lock(
      const LockMode locks) {
    return locks == LockMode::TurnTwoItem || locks == LockMode::FullCombined;
  }

  [[nodiscard]] static constexpr bool item_locked_on_turn(
      const LockMode locks, const int turn) {
    // TurnTwoItem and FullCombined begin on the player's second turn and remain
    // active afterward. FullItem is the explicit always-on synthetic fixture.
    // Repository timing contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Historical aggregate correction: https://github.com/FlareZ123/pokemon-sims/issues/2247
    return locks == LockMode::FullItem ||
           (uses_turn_two_item_lock(locks) && turn >= 2);
  }

  [[nodiscard]] static constexpr bool uses_path_style_ability_lock(
      const LockMode locks) {
    // Path to the Peak suppresses Rule Box Pokémon Abilities while its lock is live:
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    return locks == LockMode::FullRuleBoxAbility ||
           locks == LockMode::FullCombined;
  }
};

#endif  // SIM_TRACE_LOCK_MODE_POLICY_INCLUDED

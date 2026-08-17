  class GarbodorScenarioPolicy final {
   public:
    inline static constexpr std::string_view kShakeScenarioPrefix =
        "garbodor-shake-ability-lock/";

    [[nodiscard]] static constexpr bool matches(const std::string_view label) {
      // Keep scenario identity beside the activation timing it selects so callers
      // do not duplicate or reinterpret the Garbodor scenario prefix.
      // C++ string_view starts_with semantics: https://eel.is/c++draft/string.view
      return label.starts_with(kShakeScenarioPrefix);
    }

    [[nodiscard]] static constexpr int lock_activation_turn(
        const bool simulator_goes_first) {
      return simulator_goes_first ? 2 : 1;
    }

    [[nodiscard]] static constexpr bool activation_turn_reached(
        const int turn, const bool simulator_goes_first) {
      return turn >= lock_activation_turn(simulator_goes_first);
    }

    [[nodiscard]] static constexpr bool active(
        const std::string_view label, const int turn,
        const bool simulator_goes_first) {
      return matches(label) && activation_turn_reached(turn, simulator_goes_first);
    }
  };

  class GarbodorAbilityLockPolicy final {
   public:
    [[nodiscard]] static constexpr bool garbotoxin_locked(
        const bool lock_present, const bool lock_removed) {
      // Garbotoxin removes other Pokemon Abilities only while its modeled lock is
      // present and has not been removed by the shared lock-removal state:
      // https://api.pokemontcg.io/v2/cards/xy9-57
      // https://github.com/FlareZ123/pokemon-sims/issues/2808
      return lock_present && !lock_removed;
    }

    [[nodiscard]] static constexpr bool rule_box_ability_available(
        const bool is_rule_box, const bool rule_box_locked) {
      // Path to the Peak affects Pokemon with a Rule Box while it remains active:
      // https://api.pokemontcg.io/v2/cards/swsh6-148
      // https://github.com/FlareZ123/pokemon-sims/issues/2808
      return !(is_rule_box && rule_box_locked);
    }

    [[nodiscard]] static constexpr bool ability_available(
        const bool garbotoxin_locked, const bool rule_box_available) {
      // The modeled Garbotoxin and Path checks are independent lock axes; an Ability
      // is available only when both applicable gates admit it:
      // https://api.pokemontcg.io/v2/cards/xy9-57
      // https://api.pokemontcg.io/v2/cards/swsh6-148
      // https://github.com/FlareZ123/pokemon-sims/issues/2808
      return !garbotoxin_locked && rule_box_available;
    }

    [[nodiscard]] static constexpr bool ability_available_from_lock_state(
        const bool lock_present, const bool lock_removed,
        const bool is_rule_box, const bool rule_box_locked) {
      // Compose already-modeled Garbotoxin and Rule Box facts in one policy seam so
      // Engine callers do not duplicate the same two-gate expression.
      // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
      // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
      // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#shared-policy-owners
      return ability_available(
          garbotoxin_locked(lock_present, lock_removed),
          rule_box_ability_available(is_rule_box, rule_box_locked));
    }
  };

  [[nodiscard]] bool garbodor_shake_scenario() const {
    return GarbodorScenarioPolicy::matches(scenario_.label);
  }

  [[nodiscard]] int garbodor_lock_activation_turn() const {
    return GarbodorScenarioPolicy::lock_activation_turn(scenario_.going_first);
  }

  [[nodiscard]] bool garbodor_lock_removed() const {
    // The registered Garbodor model reuses the shared lock-removal state:
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return state_.path_lock_removed;
  }

  [[nodiscard]] bool garbodor_lock_established() const {
    // Boost Shake evolves a Pokemon at the end of the opponent's turn. The modeled
    // opponent uses it on its first turn to establish Garbodor, then keeps a Tool on
    // Garbodor for Garbotoxin. Going first leaves our T1 before that opponent turn;
    // going second means the opponent's setup turn has already happened:
    // https://api.pokemontcg.io/v2/cards/swsh7-142
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return GarbodorScenarioPolicy::activation_turn_reached(
        state_.turn, scenario_.going_first);
  }

  [[nodiscard]] bool garbodor_lock_present() const {
    return GarbodorScenarioPolicy::active(
        scenario_.label, state_.turn, scenario_.going_first);
  }

  [[nodiscard]] bool garbodor_abilities_locked() const {
    return GarbodorAbilityLockPolicy::garbotoxin_locked(
        garbodor_lock_present(), garbodor_lock_removed());
  }

  [[nodiscard]] bool rule_box_ability_available(const Card card) const {
    return GarbodorAbilityLockPolicy::rule_box_ability_available(
        is_rule_box_pokemon(card), rule_box_abilities_locked());
  }

  [[nodiscard]] bool ability_available_for_pokemon_garbodor(const Card card) const {
    // Reuse the canonical Engine gates so this composition does not duplicate
    // Garbodor and Rule Box lock-state extraction already owned by those helpers.
    // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Scenario contract: https://github.com/FlareZ123/pokemon-sims/issues/2808
    return GarbodorAbilityLockPolicy::ability_available(
        garbodor_abilities_locked(), rule_box_ability_available(card));
  }
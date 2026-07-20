  int issue_1152_legacy_star_turn_ = 0;

  bool use_legacy_star_issue_1152() {
    const bool used = use_legacy_star();
    issue_1152_legacy_star_turn_ = used ? state_.turn : 0;
    return used;
  }

  void choose_supporter_issue_1152() {
    choose_supporter();
    if (issue_1152_legacy_star_turn_ != state_.turn) {
      issue_1152_legacy_star_turn_ = 0;
      return;
    }
    issue_1152_legacy_star_turn_ = 0;
    if (!late_supporter_quick_ball_latias_completion_available()) return;

    // Legacy Star may recover a Supporter that searches Quick Ball after the
    // previous Item fixed point. Replay that newly live Item only through the
    // existing cost, GGF, payload, Bench, lock, Latias, Ability, and retreat gates:
    // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Core Supporter, Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1152
    play_items_until_stable(true);
    play_basics_from_hand();
  }

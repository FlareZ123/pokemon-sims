  bool promote_best_benched_vstar() {
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    std::swap(*state_.active, *target);
    return true;
  }

  bool retreat_to_benched_vstar_with_latias() {
    if (!need_active_vstar() || !can_free_retreat_with_latias()) return false;
    // Latias ex removes the Basic Active's Retreat Cost. Promote the Benched
    // Regidrago VSTAR that best satisfies Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (!promote_best_benched_vstar()) return false;
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT",
          "Latias ex gives the Basic Active no Retreat Cost; promoted the "
          "best-powered Benched Regidrago VSTAR.");
    return true;
  }

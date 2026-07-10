  bool use_fss() {
    const bool latias_solves_only_missing_axis =
        !state_.vstar_power_used && need_active_vstar() && !need_regi() &&
        !need_vstar() && !need_energy() && !need_payload() &&
        can_free_retreat_with_latias();

    if (latias_solves_only_missing_axis) {
      const auto live_holder = [](const Pokemon& pokemon) {
        return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
      };
      const bool forest_seal_stone_is_live =
          (state_.active && live_holder(*state_.active)) ||
          std::any_of(state_.bench.begin(), state_.bench.end(), live_holder);
      if (forest_seal_stone_is_live) {
        // Latias ex already gives the Basic Active no Retreat Cost, and retreating once
        // during the turn can promote the complete Benched Regidrago VSTAR. Preserve
        // the one-per-game Star Alchemy resource when that is the sole missing axis:
        // https://api.pokemontcg.io/v2/cards/sv8-76
        // https://www.pokemon.com/us/pokemon-tcg/rules
        // https://api.pokemontcg.io/v2/cards/swsh12-156
        trace("HOLD VSTAR POWER", "R-LATIAS-01; R-GAME-RETREAT; R-FSS-02; R-VSTAR-01",
              "Latias ex already supplies the complete free-retreat route to the Benched Regidrago VSTAR.");
        return false;
      }
    }

    return use_fss_latias_original();
  }

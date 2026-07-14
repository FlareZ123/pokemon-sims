  bool use_fss() {
    const bool incomplete_tate_blender_route =
        !state_.vstar_power_used && supporter_allowed() && need_active_vstar() &&
        !need_energy() && !benched_vstar_promotion_ready() && need_payload() &&
        !item_locked() && hand_count(Card::BrilliantBlender) > 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck() &&
        !can_free_retreat_with_latias() && hand_count(Card::TateLiza) == 0 &&
        might_be_unseen(Card::TateLiza);

    if (incomplete_tate_blender_route) {
      // Star Alchemy is a one-per-game VSTAR Power. Tate & Liza can promote the
      // Benched VSTAR and Brilliant Blender can supply a same-turn payload, but the
      // selected attacker must already pay Apex Dragon's GGF before that search is
      // a complete route. Preserve the VSTAR Power while the target is incomplete:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sm7-148
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      trace("HOLD VSTAR POWER", "R-FSS-02; R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Star Alchemy because Tate would promote an Energy-incomplete Regidrago VSTAR.");
      return false;
    }

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

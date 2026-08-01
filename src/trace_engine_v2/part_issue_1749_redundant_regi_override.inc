  bool issue_1749_existing_regi_ready_route_without_backup() const {
    // A second Regidrago V is optional Bench placement and has no setup effect
    // when the existing Active Regidrago V already has a complete current-turn
    // route through Star Alchemy, the held final Energy, evolution, and Blender:
    // Regidrago V / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Hisuian Heavy Ball / full Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Bench and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route bug / confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/1749 https://github.com/FlareZ123/pokemon-sims/issues/2068
    if (!prizes_known() || state_.turn_ended || item_locked() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        hand_count(Card::RegidragoVstar) == 0 ||
        grass_needed() + fire_needed() > 1) {
      return false;
    }

    const int missing_energy = grass_needed() + fire_needed();
    const bool energy_route_live = missing_energy == 0 ||
        (!state_.manual_energy_used &&
         ((grass_needed() == 1 && hand_count(Card::Grass) > 0) ||
          (fire_needed() == 1 && hand_count(Card::Fire) > 0)));
    if (!energy_route_live) return false;

    const auto live_fss_holder = [this](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone &&
          is_pokemon_v(pokemon.card) &&
          ability_available_for_pokemon(pokemon.card);
    };
    const bool live_star_alchemy =
        (state_.active && live_fss_holder(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder);
    const bool blender_route_live = hand_count(Card::BrilliantBlender) > 0 ||
        (!state_.vstar_power_used && live_star_alchemy &&
         deck_count_after_search_started(Card::BrilliantBlender) > 0);

    return blender_route_live && can_play_payload_this_turn() &&
        payload_might_be_in_deck();
  }

  int in_play_count_issue1749(const Card card) const {
    if (card == Card::RegidragoV &&
        issue_1749_existing_regi_ready_route_without_backup()) {
      // The generic branch benches only while fewer than two Regidrago V are in
      // play. Returning its threshold here suppresses that one dominated action
      // while every incomplete or K0 state delegates to the physical count:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013.inc
      // https://github.com/FlareZ123/pokemon-sims/issues/1749
      return 2;
    }
    return in_play_count(card);
  }

#define in_play_count in_play_count_issue1749

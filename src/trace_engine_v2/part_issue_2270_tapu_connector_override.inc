  bool needs_tapu_connector() {
    if (needs_tapu_connector_issue2270_original()) return true;

    // The original complete-route wrapper requires its pre-#2270 base predicate to
    // fire before it consults the K1 Arven payload bridge. The live Latias promotion
    // is the one additional current-turn route: Wonder Tag finds Arven, Arven finds
    // Quick Ball, Quick Ball discards the held Dragon and searches Latias ex, then
    // Skyliner supplies the Basic Active's free Retreat into the GGF VSTAR.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, Bench, Ability, and Retreat procedure:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Complete-route and strict-JIT policy:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
    return wonder_tag_arven_held_payload_route_available() &&
           original_wonder_tag_route_has_live_target();
  }

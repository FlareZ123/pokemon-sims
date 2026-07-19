  bool bench_tapu_if_useful() {
    // The active empty-deck guard must consult the complete-route-aware connector
    // predicate before delegating to the existing physical bench and search checks:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/989
    if (!needs_tapu_connector()) return false;
    return bench_tapu_if_useful_issue989_empty_deck_original();
  }

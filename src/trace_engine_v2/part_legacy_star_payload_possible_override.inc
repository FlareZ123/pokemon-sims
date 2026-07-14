  bool use_legacy_star() {
    // Legacy Star can create a new strict-JIT payload only by discarding cards from
    // the deck. Preserve the shared once-per-game VSTAR Power when the public empty
    // deck state or K1 knowledge proves no modeled Dragon payload can remain:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    if (need_payload() && !payload_might_be_in_deck()) return false;
    return use_legacy_star_payload_possible_original();
  }

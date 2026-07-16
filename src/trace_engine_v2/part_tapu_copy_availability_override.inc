  bool tapu_connector_copy_aware_in_play(const Card card) const {
    // Wonder Tag belongs to the individual Tapu Lele-GX played from hand onto the
    // Bench. Species-level board presence cannot suppress another physical copy;
    // the surrounding held-card, fixed-list, deck, Bench-space, and live-target
    // checks still prove whether that second copy is available and useful:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/issues/746
    return card == Card::TapuLeleGX ? false : in_play(card);
  }

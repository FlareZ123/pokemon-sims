  bool tapu_connector_copy_aware_in_play(const Card card) const {
    if (card != Card::TapuLeleGX) return in_play(card);

    // Wonder Tag belongs to the individual Tapu Lele-GX played from hand onto the
    // Bench. Ignore species-level board presence only when another physical copy is
    // proven in hand or in an inspected deck. K0 uncertainty alone must not displace
    // a stronger deterministic route such as Team Yell's Cheer into Latias ex:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/746
    const bool copy_proven_in_hand = hand_count(Card::TapuLeleGX) > 0;
    const bool copy_proven_in_deck = deck_seen_ &&
        std::find(state_.deck.begin(), state_.deck.end(), Card::TapuLeleGX) !=
            state_.deck.end();
    return copy_proven_in_hand || copy_proven_in_deck
        ? false
        : in_play(Card::TapuLeleGX);
  }

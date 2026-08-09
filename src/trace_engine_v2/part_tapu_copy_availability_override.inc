  bool has_redundant_tapu_connector_copy(const Card card) const {
    return card == Card::TapuLeleGX && has_known_hand_or_deck_copy(card);
  }

  bool tapu_connector_copy_aware_in_play(const Card card) const {
    // Wonder Tag belongs to the individual Tapu Lele-GX played from hand onto the
    // Bench. Ignore species-level board presence only when another physical copy is
    // proven in hand or in an inspected deck. K0 uncertainty alone must not displace
    // a stronger deterministic route such as Team Yell's Cheer into Latias ex:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/746
    return !has_redundant_tapu_connector_copy(card) && in_play(card);
  }

// Preserve the legacy Crobat selector under a private name so the confirmed
// Forest Seal Stone hand-compression policy can replace it after part_006.inc:
// https://api.pokemontcg.io/v2/cards/swsh12-156
// https://api.pokemontcg.io/v2/cards/swsh3-104
// https://github.com/FlareZ123/pokemon-sims/issues/1726
#define bench_crobat_if_useful bench_crobat_if_useful_issue_1726_original

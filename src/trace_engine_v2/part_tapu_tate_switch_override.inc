  bool needs_tapu_connector() {
    if (needs_tapu_connector_original()) return true;

    // Wonder Tag can search Tate & Liza, whose switch mode immediately promotes a
    // Benched Regidrago VSTAR. Spend the Bench slot and Ability only when the actual
    // promotion target already pays Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // A player may use the still-open Supporter play before attacking:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool tate_completes_active_axis = need_active_vstar() &&
        benched_vstar_promotion_ready() && !need_payload() &&
        !can_free_retreat_with_latias() && hand_count(Card::TateLiza) == 0 &&
        might_be_unseen(Card::TateLiza);
    return supporter_allowed() && ability_available_for_pokemon(Card::TapuLeleGX) &&
           tate_completes_active_axis;
  }

  bool bench_tapu_if_useful() {
    if (hand_count(Card::TapuLeleGX) == 0 || in_play(Card::TapuLeleGX) ||
        bench_space() <= 0 || !needs_tapu_connector()) {
      return false;
    }
    return bench_from_hand(Card::TapuLeleGX, true);
  }

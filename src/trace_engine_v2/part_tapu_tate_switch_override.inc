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
    // When the payload is still held, a payable one-discard Item may establish that
    // axis after Tate switches. Wonder Tag must leave a card in deck for the Item's
    // later required search, so the pre-search deck needs at least two cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_tate_blender_tate_override.inc#L1-L22
    const bool payload_axis_completable = !need_payload() ||
        (state_.deck.size() > 1U && has_live_one_discard_hand_payload_line());
    const bool tate_completes_active_axis = need_active_vstar() &&
        benched_vstar_promotion_ready() && payload_axis_completable &&
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

  bool original_wonder_tag_route_has_live_target() {
    // Wonder Tag searches a Supporter. At K1, the Supporter that justifies spending
    // Tapu Lele-GX and a Bench slot must actually remain in the inspected deck;
    // K0 keeps fixed-list uncertainty through might_be_unseen():
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool live_t1_steven = state_.turn == 1 && !scenario_.going_first &&
        should_play_steven() && hand_count(Card::StevensResolve) == 0 &&
        might_be_unseen(Card::StevensResolve);
    // Steven's Resolve searches up to 3 cards and ends the turn:
    // https://api.pokemontcg.io/v2/cards/sm7-145

    const bool live_direct_crispin = need_energy() &&
        hand_count(Card::Crispin) == 0 && crispin_can_advance_energy_axis() &&
        might_be_unseen(Card::Crispin);
    // Crispin advances the Energy axis only while a needed Basic Energy remains:
    // https://api.pokemontcg.io/v2/cards/sv7-133

    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_might_be_in_deck =
        (grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
        (fire_needed() == 1 && might_be_unseen(Card::Fire));
    const bool live_arven_vessel_energy = need_energy() && exactly_one_energy_missing &&
        !state_.manual_energy_used && !item_locked() && needed_energy_might_be_in_deck &&
        hand_count(Card::Arven) == 0 && might_be_unseen(Card::Arven) &&
        might_be_unseen(Card::EarthenVessel) &&
        is_legal_cost_available(can_play_payload_this_turn(), true);
    // Arven may find Earthen Vessel, whose paid search can put the final Basic
    // Energy into hand for the unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules

    const bool live_burnet = can_play_payload_this_turn() &&
        payload_might_be_in_deck() && hand_count(Card::ProfessorBurnet) == 0 &&
        !has_live_blender_payload_line() && might_be_unseen(Card::ProfessorBurnet);
    // Professor Burnet searches and discards up to 2 deck cards:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26

    const bool direct_ultra_vstar_route = payable_ultra_ball_vstar_route();
    const bool live_arven_vstar = need_vstar() && hand_count(Card::Arven) == 0 &&
        hand_count(Card::EvolutionIncense) == 0 &&
        hand_count(Card::MysteriousTreasure) == 0 && !direct_ultra_vstar_route &&
        might_be_unseen(Card::Arven);
    // Arven is the Supporter target for the modeled Item-to-VSTAR route:
    // https://api.pokemontcg.io/v2/cards/sv1-166

    const bool live_gladion_axis = prizes_known() && might_be_unseen(Card::Gladion) &&
        (final_prized_energy_for_gladion().has_value() ||
         (need_vstar() && !might_be_unseen(Card::RegidragoVstar) &&
          known_prize(Card::RegidragoVstar)));
    // Gladion remains a live Wonder Tag target when K1 proves the exact missing
    // VSTAR or final attachable Energy is in the Prize cards:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

    return live_t1_steven || live_direct_crispin || live_arven_vessel_energy ||
           live_burnet || live_arven_vstar || live_gladion_axis;
  }

  bool needs_tapu_connector() {
    if (needs_tapu_connector_original() && original_wonder_tag_route_has_live_target()) {
      return true;
    }

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

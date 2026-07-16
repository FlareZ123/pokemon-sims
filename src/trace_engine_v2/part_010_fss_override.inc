  Card fss_target_after_search_started() const {
    // Star Alchemy can search Tate & Liza to promote a GGF-complete Benched
    // Regidrago VSTAR. A payable held one-discard Item can establish the strict-JIT
    // payload afterward, provided Tate leaves at least one card for that Item's
    // required deck search:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_tate_blender_tate_override.inc#L1-L22
    const bool tate_held_payload_route = need_active_vstar() &&
        benched_vstar_promotion_ready() && !can_free_retreat_with_latias() &&
        supporter_allowed() && need_payload() && state_.deck.size() > 1U &&
        has_live_one_discard_hand_payload_line() &&
        deck_count_after_search_started(Card::TateLiza) > 0;
    if (tate_held_payload_route) return Card::TateLiza;

    const Card original = fss_target_after_search_started_original();

    const bool needed_energy_in_deck =
        (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);
    const bool oricorio_in_deck =
        bench_space() > 0 && !in_play(Card::Oricorio) &&
        hand_count(Card::Oricorio) == 0 &&
        deck_count_after_search_started(Card::Oricorio) > 0;
    // #616 defines the shared exact-hand test for whether a held Dragon has a legal
    // same-turn Item discard route. Star Alchemy should take Oricorio when Vital
    // Dance supplies the final Energy and preserves Professor Burnet's one Supporter
    // play for the only live deck-to-discard payload bridge:
    // Forest Seal Stone https://api.pokemontcg.io/v2/cards/swsh12-156
    // Oricorio https://api.pokemontcg.io/v2/cards/sm2-55
    // Crispin https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
    // Shared outlet policy https://github.com/FlareZ123/pokemon-sims/issues/616
    // Parent route https://github.com/FlareZ123/pokemon-sims/issues/187
    // Core turn limits https://www.pokemon.com/us/pokemon-tcg/rules
    // Decision priority https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool oricorio_preserves_burnet_with_stranded_payload =
        original == Card::Crispin && needed_energy_in_deck && oricorio_in_deck &&
        oricorio_preserves_burnet_with_current_hand();
    if (oricorio_preserves_burnet_with_stranded_payload) return Card::Oricorio;

    if (original != Card::Crispin || supporter_allowed()) return original;

    // Crispin is a Supporter, so it cannot resolve after Arven has already used the
    // turn's one Supporter play. Star Alchemy may instead take any card, including
    // the missing Basic Energy for the still-unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (need_energy() && can_fetch_oricorio_for_compression()) return Card::Oricorio;
    if (need_energy() && deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        is_legal_cost_available(can_play_payload_this_turn(), true)) {
      return Card::EarthenVessel;
    }
    if (need_energy() && !state_.manual_energy_used) {
      if (grass_needed() > 0 && deck_count_after_search_started(Card::Grass) > 0) return Card::Grass;
      if (fire_needed() > 0 && deck_count_after_search_started(Card::Fire) > 0) return Card::Fire;
    }
    if (deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    if (deck_count_after_search_started(Card::MysteriousTreasure) > 0) return Card::MysteriousTreasure;
    if (deck_count_after_search_started(Card::QuickBall) > 0) return Card::QuickBall;
    return Card::Serena;
  }

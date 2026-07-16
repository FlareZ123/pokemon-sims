  bool ultra_ball_one_discard_outlet_has_post_payload_target() const {
    // K0 preserves fixed-list uncertainty. At K1, remove the Dragon that Ultra Ball
    // will fetch and require each held search Item to retain a target from its printed
    // class. A Trainer known to have no effect cannot be played:
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    if (!deck_seen_) {
      return hand_count(Card::MysteriousTreasure) > 0 ||
             hand_count(Card::QuickBall) > 0 ||
             hand_count(Card::EarthenVessel) > 0;
    }

    std::optional<Card> planned_payload;
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX}) {
      if (std::find(state_.deck.begin(), state_.deck.end(), payload) != state_.deck.end()) {
        planned_payload = payload;
        break;
      }
    }
    if (!planned_payload) return false;

    std::vector<Card> post_search_deck = state_.deck;
    const auto payload_it = std::find(post_search_deck.begin(), post_search_deck.end(),
                                      *planned_payload);
    if (payload_it != post_search_deck.end()) post_search_deck.erase(payload_it);

    const bool mysterious_outlet = hand_count(Card::MysteriousTreasure) > 0 &&
        std::any_of(post_search_deck.begin(), post_search_deck.end(), is_dragon_or_psychic);
    const bool quick_outlet = hand_count(Card::QuickBall) > 0 &&
        std::any_of(post_search_deck.begin(), post_search_deck.end(), is_basic);
    const bool vessel_outlet = hand_count(Card::EarthenVessel) > 0 &&
        (std::find(post_search_deck.begin(), post_search_deck.end(), Card::Grass) !=
             post_search_deck.end() ||
         std::find(post_search_deck.begin(), post_search_deck.end(), Card::Fire) !=
             post_search_deck.end());
    return mysterious_outlet || quick_outlet || vessel_outlet;
  }

  bool play_ultra_ball(const bool permit_payload) {
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_regi = need_regi() && bench_space() > 0 &&
                           might_be_unseen(Card::RegidragoV);
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
                            might_be_unseen(Card::RegidragoVstar);
    const bool want_latias = need_active_vstar() && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 && might_be_unseen(Card::LatiasEx);
    const bool want_oricorio = need_energy() && hand_count(Card::Oricorio) == 0 &&
        !in_play(Card::Oricorio) && bench_space() > 0 && needs_oricorio_connector() &&
        might_be_unseen(Card::Oricorio);
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 &&
        !in_play(Card::TapuLeleGX) && bench_space() > 0 && needs_tapu_connector() &&
        might_be_unseen(Card::TapuLeleGX);
    const bool payload_might_be_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar) ||
        might_be_unseen(Card::DialgaGX);
    const bool route_would_search_payload = need_payload() && payload_might_be_searchable &&
        !can_jit_discard && !want_regi && !want_vstar && !want_latias &&
        !want_oricorio && !want_tapu;
    const bool one_discard_outlet_held = hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0;
    const bool serena_outlet = supporter_allowed() && hand_count(Card::Serena) > 0;

    // Apex Dragon needs the fetched Dragon in discard. Preserve the existing exact
    // double-Ultra simulation and Serena route, while rejecting a K1 search route
    // whose promised one-discard Item has no legal post-search target:
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (route_would_search_payload && one_discard_outlet_held && !serena_outlet &&
        !second_ultra_ball_can_discard_fetched_payload() &&
        !ultra_ball_one_discard_outlet_has_post_payload_target()) {
      return false;
    }
    return play_ultra_ball_target_class_original(permit_payload);
  }

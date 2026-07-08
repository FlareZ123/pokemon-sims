    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::QuickBall) == 0) return false;
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_regi = need_regi() && bench_space() > 0;
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 && needs_tapu_connector();
    const bool want_oricorio = hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0 && needs_oricorio_connector();
    // Quick Ball pays its discard before the fetched Basic reaches hand. A later
    // Ultra Ball route is live only when its own two-card cost remains payable:
    // https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool want_payload = need_payload() && !can_jit_discard &&
                              has_payable_payload_outlet_after_costed_search(Card::QuickBall);
    if (!can_jit_discard && !want_regi && !want_latias && !want_tapu && !want_oricorio && !want_payload) return false;
    const auto cost = choose_discard(can_jit_discard, true, true, Card::QuickBall);
    if (!cost) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(*cost, "Quick Ball cost", "R-QB-01");
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh1-179
    record_deck_search_knowledge("Quick Ball");
    const std::array<Card, 6> targets{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::DialgaGX, Card::MawileGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (target == Card::Oricorio && want_oricorio) ||

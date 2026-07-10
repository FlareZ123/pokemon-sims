    return Card::RegidragoV;
  }

  Card fallback_ultra_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::MawileGX, Card::Dipplin}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;
    // is_payload is the A/S readiness set. Dipplin is a legal Dragon search
    // target, while maintainer policy keeps it out of payload-discard routing:
    // https://api.pokemontcg.io/v2/cards/sv6-127
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Mysterious Treasure searches only the deck after paying its discard cost.
    // K0 keeps fixed-list targets plausible; K1 requires the inspected deck to
    // contain the desired Psychic or Dragon Pokémon before spending both cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool want_regi = need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV);
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
                            might_be_unseen(Card::RegidragoVstar);
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
                             might_be_unseen(Card::LatiasEx);
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 &&
                           needs_tapu_connector() && might_be_unseen(Card::TapuLeleGX);
    const bool want_oricorio = hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0 &&
                                needs_oricorio_connector() && might_be_unseen(Card::Oricorio);
    // Treasure must pay its discard before the fetched payload reaches hand. A later
    // Ultra Ball route is live only when its own two-card cost remains payable:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool want_payload = need_payload() && !can_jit_discard &&
                              has_payable_payload_outlet_after_costed_search(Card::MysteriousTreasure);
    if (!can_jit_discard && !want_regi && !want_vstar && !want_latias && !want_tapu && !want_oricorio && !want_payload) return false;
    // The Mysterious Treasure being played cannot discard itself. A distinct held
    // Ultra Ball remains a legal card from hand for Treasure's discard cost, even
    // when that Ultra Ball is not itself payable because it needs two other cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const auto cost = choose_discard(can_jit_discard, true, true, Card::MysteriousTreasure);
    if (!cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(*cost, "Mysterious Treasure cost", "R-MT-01");

    // Card text source: https://api.pokemontcg.io/v2/cards/sm6-113
    // The deck is now being legally searched. Select a legal target after inspecting it.
    record_deck_search_knowledge("Mysterious Treasure");
    const std::array<Card, 9> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::TapuLeleGX,
                                      Card::Oricorio, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (want_payload && is_payload(target));
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM", "Searched a Psychic or Dragon Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    // Once this search proves the needed Regidrago is prized, Tapu Lele-GX can be
    // searched and Benched to find Gladion with Wonder Tag in the same turn:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm4-95
    const bool prized_regi_axis = deck_count_after_search_started(Card::Gladion) > 0 &&
        ((need_regi() && deck_count_after_search_started(Card::RegidragoV) == 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) == 0 &&
          prize_count_after_reveal(Card::RegidragoVstar) > 0));
    const bool found_tapu_for_gladion = prized_regi_axis && bench_space() > 0 &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        move_deck_to_hand(Card::TapuLeleGX);
    const Card fallback = found_tapu_for_gladion ? Card::TapuLeleGX : fallback_mysterious_target_after_search_started();
    const bool found_fallback = found_tapu_for_gladion || move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Psychic or Dragon Pokémon remained after search.");
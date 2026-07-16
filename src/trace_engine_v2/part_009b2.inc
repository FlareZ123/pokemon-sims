                           (target == Card::DialgaGX && want_payload);
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM", "Searched a Basic Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    // A completed Quick Ball search reveals whether the missing Regidrago remains
    // in deck. A proved Prize target makes Tapu Lele-GX -> Gladion a live fallback:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm4-95
    const bool prized_regi_axis = deck_count_after_search_started(Card::Gladion) > 0 &&
        ((need_regi() && deck_count_after_search_started(Card::RegidragoV) == 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) == 0 &&
          prize_count_after_reveal(Card::RegidragoVstar) > 0));
    const bool found_tapu_for_gladion = prized_regi_axis && bench_space() > 0 &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        move_deck_to_hand(Card::TapuLeleGX);
    const Card fallback = found_tapu_for_gladion ? Card::TapuLeleGX : fallback_quick_ball_target_after_search_started();
    const bool found_fallback = found_tapu_for_gladion || move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Basic Pokémon remained after search.");
    return true;
  }

  bool ultra_ball_has_live_post_payload_outlet() const {
    const bool serena_outlet = supporter_allowed() && hand_count(Card::Serena) > 0;
    const bool second_ultra_outlet = hand_count(Card::UltraBall) > 1;

    // K0 preserves fixed-list uncertainty. At K1, remove the exact Dragon payload
    // that Ultra Ball will fetch and require each promised second search Item to
    // retain a target from its printed class:
    // Mysterious Treasure https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel https://api.pokemontcg.io/v2/cards/sv4-163
    // Ultra Ball https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    if (!deck_seen_) {
      return hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::QuickBall) > 0 ||
             hand_count(Card::EarthenVessel) > 0 || second_ultra_outlet || serena_outlet;
    }

    std::optional<Card> planned_payload;
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX}) {
      if (std::find(state_.deck.begin(), state_.deck.end(), payload) != state_.deck.end()) {
        planned_payload = payload;
        break;
      }
    }
    if (!planned_payload) return second_ultra_outlet || serena_outlet;

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
    return mysterious_outlet || quick_outlet || vessel_outlet || second_ultra_outlet ||
           serena_outlet;
  }

  bool play_ultra_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
    // Ultra Ball discards 2 other cards from hand before searching a Pokémon: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // The current-turn payload condition is simulator policy, which permits the printed discard cost only when it advances strict-JIT readiness.
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                  std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_regi = need_regi() && bench_space() > 0;
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                              !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool want_oricorio = need_energy() && hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
                                bench_space() > 0 && needs_oricorio_connector();
    // Tapu Lele-GX may turn this Pokémon search into a live Supporter connector
    // through Wonder Tag when it is Benched from hand: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
                            bench_space() > 0 && needs_tapu_connector();
    // A fetched Dragon payload can immediately become a strict-JIT payload only when
    // a distinct surviving hand-discard outlet remains legal after the search.
    // Apex Dragon requires the Dragon Pokémon in discard:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool has_live_hand_payload_outlet = ultra_ball_has_live_post_payload_outlet();
    const bool want_payload = need_payload() && !can_jit_discard && has_live_hand_payload_outlet;
    if (!want_regi && !want_vstar && !want_latias && !want_oricorio && !want_tapu && !want_payload && !can_jit_discard) return false;

    const auto first = choose_discard(can_jit_discard, true);
    if (!first) return false;
    remove_one(state_.hand, *first);

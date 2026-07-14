  bool second_ultra_ball_can_discard_fetched_payload() {
    // Ultra Ball requires two other cards each time it is played. A first Ultra Ball
    // can only fetch a strict-JIT payload for another Ultra Ball when the second play
    // can discard that payload plus one further policy-legal hand card:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (hand_count(Card::UltraBall) < 2) return false;
    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };

    // The first Ultra Ball being simulated cannot pay its own cost, while either of
    // two other held copies may be selected as a legal cost card:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const auto first_cost = choose_discard(false, true, true, Card::UltraBall);
    if (!first_cost || !remove_one(state_.hand, *first_cost)) {
      restore_hand();
      return false;
    }
    const auto second_cost = choose_discard(false, true, true, Card::UltraBall);
    if (!second_cost || !remove_one(state_.hand, *second_cost) || !remove_one(state_.hand, Card::UltraBall)) {
      restore_hand();
      return false;
    }

    // The exact Dragon identity does not affect discard eligibility. Mega Dragonite ex
    // is a modeled Dragon payload and represents the card fetched by the first search:
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    state_.hand.push_back(Card::MegaDragonite);
    // A second Ultra Ball must still be in hand after the first Ultra Ball and both of
    // its discard costs have left the hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (hand_count(Card::UltraBall) == 0) {
      restore_hand();
      return false;
    }
    const auto fetched_payload = choose_discard(true, true, true, Card::UltraBall);
    if (!fetched_payload || !is_payload(*fetched_payload) || !remove_one(state_.hand, *fetched_payload)) {
      restore_hand();
      return false;
    }
    const bool payable = choose_discard(false, true, true, Card::UltraBall).has_value();
    restore_hand();
    return payable;
  }

  bool ultra_ball_can_discard_fetched_incense_payload() {
    if (hand_count(Card::UltraBall) == 0 || hand_count(Card::EvolutionIncense) == 0) return false;
    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };

    // Evaluate the exact hand after Evolution Incense leaves play and fetches an
    // Evolution Dragon. Ultra Ball must reserve itself and discard the fetched payload
    // plus one distinct remaining card:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    if (!remove_one(state_.hand, Card::EvolutionIncense)) {
      restore_hand();
      return false;
    }
    state_.hand.push_back(Card::MegaDragonite);
    if (!remove_one(state_.hand, Card::MegaDragonite)) {
      restore_hand();
      return false;
    }

    const bool payload_is_only_missing_axis = need_payload() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_active_vstar();
    // Once the fetched Dragon will complete the sole missing Apex Dragon axis, the
    // surviving Evolution Incense is a legal distinct second Ultra Ball cost:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool payable = choose_discard(false, true, true, Card::UltraBall).has_value() ||
        (payload_is_only_missing_axis && hand_count(Card::EvolutionIncense) > 0);
    restore_hand();
    return payable;
  }

  bool play_ultra_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
    // Ultra Ball discards 2 other cards from hand before searching a Pokémon: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // The current-turn payload condition is simulator policy, which permits the printed discard cost only when it advances strict-JIT readiness.
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                   std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Ultra Ball searches only the deck after paying its two-card discard cost. K0
    // keeps fixed-list Pokémon plausible; K1 requires the inspected deck to contain
    // each desired target before spending all three hand cards:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool want_regi = need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV);
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
                            might_be_unseen(Card::RegidragoVstar);
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
                             might_be_unseen(Card::LatiasEx);
    const bool want_oricorio = need_energy() && hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
                                bench_space() > 0 && needs_oricorio_connector() &&
                                might_be_unseen(Card::Oricorio);
    // Tapu Lele-GX may turn this Pokémon search into a live Supporter connector
    // through Wonder Tag when it is Benched from hand: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
                           bench_space() > 0 && needs_tapu_connector() &&
                           might_be_unseen(Card::TapuLeleGX);
    // A fetched Dragon payload can immediately become a strict-JIT payload when a
    // distinct live hand-discard effect remains. Mysterious Treasure, Quick Ball,
    // and Earthen Vessel each discard from hand; Serena may discard from hand before
    // drawing, but must discard at least 1 card. Here the fetched payload becomes
    // Serena's mandatory discard, so Serena is not a zero-discard outlet.
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12-164 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool has_live_hand_payload_outlet = hand_count(Card::MysteriousTreasure) > 0 ||
                                               hand_count(Card::QuickBall) > 0 ||
                                               hand_count(Card::EarthenVessel) > 0 ||
                                               second_ultra_ball_can_discard_fetched_payload() ||
                                               (supporter_allowed() && hand_count(Card::Serena) > 0);
    const bool payload_might_be_searchable = might_be_unseen(Card::MegaDragonite) ||
                                               might_be_unseen(Card::Dragapult) ||
                                               might_be_unseen(Card::GoodraVstar) ||
                                               might_be_unseen(Card::DialgaGX);
    const bool want_payload = need_payload() && payload_might_be_searchable &&
                              !can_jit_discard && has_live_hand_payload_outlet;
    if (!want_regi && !want_vstar && !want_latias && !want_oricorio && !want_tapu && !want_payload && !can_jit_discard) return false;

    // The Ultra Ball being played cannot pay its own cost. Two other Ultra Ball
    // copies are legal first and second cost cards when all three copies are held:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const auto first = choose_discard(can_jit_discard, true, true, Card::UltraBall);
    if (!first) return false;
    if (!remove_one(state_.hand, *first)) throw std::logic_error("Ultra Ball first discard cost disappeared");
    // The Ultra Ball currently being played cannot pay its own two-card cost, while
    // a distinct remaining Ultra Ball can. Preserve that distinction in the second
    // selection: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool remaining_incense_is_ready_now_second_cost = is_payload(*first) && need_payload() &&
        !need_regi() && !need_vstar() && !need_energy() && !need_active_vstar() &&
        hand_count(Card::EvolutionIncense) > 0;
    // After Evolution Incense has fetched the sole missing Dragon payload, its
    // surviving duplicate is another card and may pay Ultra Ball's second cost:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const std::optional<Card> second = remaining_incense_is_ready_now_second_cost
        ? std::optional<Card>{Card::EvolutionIncense}
        : choose_discard(permit_payload && can_play_payload_this_turn() && !is_payload(*first),
                         true, true, Card::UltraBall);
    state_.hand.push_back(*first);
    if (!second) return false;

    remove_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first, "Ultra Ball cost", "R-UB-01");
    discard_from_hand(*second, "Ultra Ball cost", "R-UB-01");
    // Ultra Ball searches for any Pokémon after its two-card cost: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    record_deck_search_knowledge("Ultra Ball");

    // A fetched Dragon can become this turn's strict-JIT payload through the separate
    // hand-discard outlet selected in play_ultra_ball: https://api.pokemontcg.io/v2/cards/swsh12-136
    const std::array<Card, 9> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                                      Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                      Card::DialgaGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (want_payload && is_payload(target));
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM", "Searched a Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    const Card fallback = fallback_ultra_ball_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Pokémon remained after search.");
    return true;
  }

  bool play_evolution_incense(const bool permit_payload = false) {
    if (item_locked() || hand_count(Card::EvolutionIncense) == 0) return false;
    // Before K1, the fixed-list copy may still be in deck. After legal inspection,
    // do not spend Incense for the VSTAR axis when Regidrago VSTAR is known absent:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
                            might_be_unseen(Card::RegidragoVstar);
    // Evolution Incense can search any Evolution Pokémon. A JIT payload search is
    // live only when a separate card already in hand can discard that payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // A held Ultra Ball can discard the fetched Evolution payload plus a distinct
    // surviving card after the played Incense leaves hand:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool ultra_ball_payload_discard_outlet =
        ultra_ball_can_discard_fetched_incense_payload();
    const bool hand_payload_discard_outlet = hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        ultra_ball_payload_discard_outlet || (supporter_allowed() && hand_count(Card::Serena) > 0);
    const bool want_payload = permit_payload && can_play_payload_this_turn() && need_payload() &&
                              hand_payload_discard_outlet;
    if (!want_vstar && !want_payload) return false;

    remove_one(state_.hand, Card::EvolutionIncense);
    state_.discard.push_back(Card::EvolutionIncense);
    record_deck_search_knowledge("Evolution Incense");
    bool found = false;
    Card found_card = Card::RegidragoVstar;
    if (want_vstar && move_deck_to_hand(Card::RegidragoVstar)) {
      found = true;
      found_card = Card::RegidragoVstar;
    }
    // The modeled Evolution payloads are legal Evolution Incense targets:
    // Mega Dragonite ex https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Dragapult ex https://api.pokemontcg.io/v2/cards/sv6-130
    // Hisuian Goodra VSTAR https://api.pokemontcg.io/v2/cards/swsh11-136
    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar}) {
        if (move_deck_to_hand(payload)) {
          found = true;
          found_card = payload;
          break;
        }
      }
    }
    // Once the legal deck search has begun, any present Evolution Pokémon is a
    // valid fallback. Do not falsely fail while a Stage 2, VSTAR, or Stage 1 remains:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh11-136
    // https://api.pokemontcg.io/v2/cards/sv6-127
    if (!found) {
      for (const Card fallback : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::Dipplin}) {
        if (move_deck_to_hand(fallback)) {
          found = true;
          found_card = fallback;
          break;
        }
      }
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EVOINCENSE-01; R-GAME-ITEM", found ? "Searched an Evolution Pokémon: " + std::string(name(found_card)) + "." : "No Evolution Pokémon remained in deck.");
    return true;
  }
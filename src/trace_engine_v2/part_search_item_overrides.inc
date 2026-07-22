  bool second_ultra_ball_can_discard_fetched_payload() {
    // A K1 Ultra Ball route requires at least one Pokémon target before either copy
    // can pay its two-card requirement:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    if (!ultra_ball_has_legal_target()) return false;
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
    // A future Ultra Ball cannot be counted as a live discard outlet when K1 proves
    // that its Pokémon search has no target:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    if (!ultra_ball_has_legal_target()) return false;
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

  bool ultra_ball_has_live_post_payload_outlet() {
    const bool second_ultra_outlet = second_ultra_ball_can_discard_fetched_payload();
    const bool serena_outlet = supporter_allowed() && hand_count(Card::Serena) > 0;

    // K0 preserves fixed-list uncertainty. At K1, remove the Dragon that Ultra Ball
    // will fetch and require each promised search Item to retain a printed-class target:
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
    // Appletun is a legal Ultra Ball target and modeled Apex Dragon payload.
    // Remove the exact planned payload before proving a follow-up Item target:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1287
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX,
                               Card::Appletun}) {
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
    // through Wonder Tag when it is Benched from hand: https://api.pokemontcg.io/v2/cards/sm2-60
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
                           bench_space() > 0 && needs_tapu_connector() &&
                           might_be_unseen(Card::TapuLeleGX);
    // A fetched Dragon becomes a strict-JIT payload only when a surviving discard
    // outlet remains legal after Ultra Ball removes that Dragon from the deck:
    // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool has_live_hand_payload_outlet = ultra_ball_has_live_post_payload_outlet();
    // Ultra Ball may search any Pokémon. Appletun sv8-140 is both modeled and a
    // strict-JIT payload, so it belongs in the K0 payload-availability gate:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1254
    const bool payload_might_be_searchable = might_be_unseen(Card::MegaDragonite) ||
                                               might_be_unseen(Card::Dragapult) ||
                                               might_be_unseen(Card::GoodraVstar) ||
                                               might_be_unseen(Card::DialgaGX) ||
                                               might_be_unseen(Card::Appletun);
    const bool want_payload = need_payload() && payload_might_be_searchable &&
                              !can_jit_discard && has_live_hand_payload_outlet;
    if (!want_regi && !want_vstar && !want_latias && !want_oricorio && !want_tapu && !want_payload && !can_jit_discard) return false;

    // The Ultra Ball being played cannot pay its own cost. Two other Ultra Ball
    // copies are legal first and second cost cards when all three copies are held:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    std::optional<Card> first =
        choose_discard(can_jit_discard, true, true, Card::UltraBall);
    std::optional<Card> second;
    if (first && remove_one(state_.hand, *first)) {
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
      // When payload is the sole missing axis at K1, one distinct singleton search
      // Item may pay Ultra Ball's second cost if another remains to discard the fetched
      // Dragon. Require a second deck card so the surviving search can still begin:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      const bool payload_is_only_missing_axis = want_payload && !need_regi() &&
          !need_vstar() && !need_energy() && !need_active_vstar();
      std::optional<Card> cross_singleton_search_cost;
      if (payload_is_only_missing_axis && deck_seen_ && state_.deck.size() > 1U) {
        std::optional<Card> planned_payload;
        // Appletun must leave the projected deck before the surviving one-discard
        // search Item is selected, or its fetched copy can be counted as its own target:
        // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
        // https://api.pokemontcg.io/v2/cards/sv8-140
        // https://api.pokemontcg.io/v2/cards/sm6-113
        // https://api.pokemontcg.io/v2/cards/sv4-163
        // https://github.com/FlareZ123/pokemon-sims/issues/1287
        for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                   Card::GoodraVstar, Card::DialgaGX,
                                   Card::Appletun}) {
          if (deck_count_after_search_started(payload) > 0) {
            planned_payload = payload;
            break;
          }
        }
        const auto remains_after_payload_search = [this, planned_payload](const Card card) {
          const int removed = planned_payload && *planned_payload == card ? 1 : 0;
          return deck_count_after_search_started(card) > removed;
        };
        const bool mysterious_has_target = std::any_of(
            state_.deck.begin(), state_.deck.end(),
            [planned_payload, &remains_after_payload_search](const Card card) {
              return (!planned_payload || card != *planned_payload ||
                      remains_after_payload_search(card)) && is_dragon_or_psychic(card);
            });
        const bool quick_has_target = std::any_of(
            state_.deck.begin(), state_.deck.end(),
            [planned_payload, &remains_after_payload_search](const Card card) {
              return (!planned_payload || card != *planned_payload ||
                      remains_after_payload_search(card)) && is_basic(card);
            });
        const bool vessel_has_target = remains_after_payload_search(Card::Grass) ||
            remains_after_payload_search(Card::Fire);
        const std::array<std::pair<Card, bool>, 3> outlets{{
            {Card::MysteriousTreasure, mysterious_has_target},
            {Card::QuickBall, quick_has_target},
            {Card::EarthenVessel, vessel_has_target},
        }};
        std::optional<Card> survivor;
        for (const bool require_live_target : {true, false}) {
          for (const auto& [outlet, has_target] : outlets) {
            if (hand_count(outlet) > 0 && (!require_live_target || has_target)) {
              survivor = outlet;
              break;
            }
          }
          if (survivor) break;
        }
        if (survivor) {
          for (const auto& outlet_info : outlets) {
            const Card outlet = outlet_info.first;
            if (outlet != *survivor && hand_count(outlet) > 0) {
              cross_singleton_search_cost = outlet;
              break;
            }
          }
        }
      }
      second = remaining_incense_is_ready_now_second_cost
          ? std::optional<Card>{Card::EvolutionIncense}
          : cross_singleton_search_cost
              ? cross_singleton_search_cost
              : choose_discard(permit_payload && can_play_payload_this_turn() && !is_payload(*first),
                               true, true, Card::UltraBall);
      state_.hand.push_back(*first);
    }

    if ((!first || !second) && want_vstar) {
      const auto redundant_costs = redundant_payload_ultra_ball_vstar_costs();
      if (redundant_costs) {
        first = (*redundant_costs)[0];
        second = (*redundant_costs)[1];
      }
    }
    if (!first || !second) return false;

    remove_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first, "Ultra Ball cost", "R-UB-01");
    discard_from_hand(*second, "Ultra Ball cost", "R-UB-01");
    // Ultra Ball searches for any Pokémon after its two-card cost: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    record_deck_search_knowledge("Ultra Ball");

    // A fetched Dragon can become this turn's strict-JIT payload through the separate
    // hand-discard outlet selected in play_ultra_ball: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Appletun is a legal Pokémon target and modeled payload:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1254
    const std::array<Card, 10> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                                       Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                       Card::DialgaGX, Card::Appletun};
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
    // A public Dragon payload can already pay the held discard outlet. Evolution
    // Incense must not spend itself to fetch a second payload that advances no axis:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1337
    const bool payload_already_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_payload = permit_payload && can_play_payload_this_turn() && need_payload() &&
                              !payload_already_held && hand_payload_discard_outlet;
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
    // Appletun is a Stage 1 Evolution Pokémon and modeled Apex Dragon payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::Appletun}) {
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
    // Regidrago VSTAR https://api.pokemontcg.io/v2/cards/swsh12-136
    // Mega Dragonite ex https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Dragapult ex https://api.pokemontcg.io/v2/cards/sv6-130
    // Hisuian Goodra VSTAR https://api.pokemontcg.io/v2/cards/swsh11-136
    // Dipplin https://api.pokemontcg.io/v2/cards/sv6-127
    if (!found) {
      // Evolution Incense searches every modeled Evolution Pokémon, including
      // Appletun and Forretress ex:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sv8-140
      // https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // https://github.com/FlareZ123/pokemon-sims/issues/1255
      for (const Card fallback : {Card::RegidragoVstar, Card::MegaDragonite,
                                  Card::Dragapult, Card::GoodraVstar,
                                  Card::Appletun, Card::Dipplin,
                                  Card::ForretressEx}) {
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

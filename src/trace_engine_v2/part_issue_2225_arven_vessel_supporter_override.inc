  std::optional<Card> issue_2225_arven_vessel_payload_cost() const {
    for (const Card card : {Card::Dragapult, Card::Appletun,
                            Card::MegaDragonite, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_2225_arven_vessel_finish_available() const {
    return strict_payload_timing() && prizes_known() && supporter_allowed() &&
        !item_locked() && deck_search_available() &&
        !state_.manual_energy_used && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::Arven) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        issue_2225_arven_vessel_payload_cost().has_value();
  }

  bool play_issue_2225_arven_vessel_finish() {
    if (!issue_2225_arven_vessel_finish_available()) return false;
    const Card payload = *issue_2225_arven_vessel_payload_cost();

    // K1 is already established before this Supporter decision. Arven can spend
    // the one Supporter action on Earthen Vessel, whose mandatory one-card cost
    // discards the held Dragon payload. Vessel then searches the missing Grass for
    // the unused manual attachment, completing both unresolved axes this turn.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Gladion and Oricorio route being preempted: https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sm2-55
    // Official Supporter, Item, discard-cost, deck-search, and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, supporter contention, dynamic DCI/JIT, and earliest-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2225
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2225 route");
    if (!move_deck_to_hand(Card::EarthenVessel)) {
      throw std::logic_error("Issue-2225 Earthen Vessel target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Arven searched Earthen Vessel for the complete Energy-plus-payload route.");

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-2225 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-2225 route");
    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-2225 Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(payload)) +
              " as the current-turn payload and searched Grass Energy for the unused manual attachment.");
    return true;
  }

  bool issue_2265_arven_quick_ball_surplus_route_live() const {
    if (!prizes_known() || !supporter_allowed() ||
        hand_count(Card::Arven) == 0 || item_locked() ||
        deck_count_after_search_started(Card::QuickBall) == 0 ||
        hand_count(Card::QuickBall) > 0) {
      return false;
    }

    // Arven consumes the turn's Supporter permission, so only a non-Supporter
    // payload outlet may complete the post-search route. Brilliant Blender is held,
    // legal this turn, and a permitted Dragon payload is known to remain in deck:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // One-Supporter-per-turn and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    const bool blender_payload_route =
        hand_count(Card::BrilliantBlender) > 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck();
    if (!blender_payload_route) return false;

    // The existing Quick Ball classifier proves the rest of the exact observable
    // continuation: selected Benched VSTAR already has GGF, the Active is Basic,
    // retreat is unused, Latias ex is physically available, Skyliner is legal,
    // Bench space exists, and one held Basic Energy is dynamically surplus. Calling
    // it before Arven is safe because the helper does not require Quick Ball in hand;
    // this gate separately proves Arven can fetch that physical Item from K1 deck.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core search, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing direct-held-Quick-Ball route: https://github.com/FlareZ123/pokemon-sims/issues/1185
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    return quick_ball_final_surplus_energy_cost().has_value();
  }

  bool play_issue_2265_arven_quick_ball_surplus_route() {
    if (!issue_2265_arven_quick_ball_surplus_route_live()) return false;

    // Search only the Item required by the proven continuation. Arven says to search
    // for an Item and a Pokémon Tool; a typed deck search may choose fewer matching
    // cards, so the Tool branch can legally fail when it advances no setup axis:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Typed-search voluntary failure ruling: https://compendium.pokegym.net/compendium-bw.html#trainers-in-general
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2265 route");
    const bool found = move_deck_to_hand(Card::QuickBall);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01; P-COMPRESS-01",
          found
              ? "Arven searched Quick Ball for the final surplus-Energy Latias/Blender route."
              : "Arven legally inspected the deck, but the projected Quick Ball target disappeared.");
    return true;
  }

  void choose_supporter() {
    if (issue_2265_arven_quick_ball_surplus_route_live()) {
      // K1 proves Arven can fetch the physical Quick Ball whose existing dynamic-DCI
      // classifier admits the now-surplus Basic Energy. This visible Arven -> Quick
      // Ball -> Latias ex -> Brilliant Blender sequence completes every unresolved
      // axis this turn, so it preempts slower or non-completing Supporter choices:
      // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Supporter, Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // K1, supporter contention, dynamic DCI/JIT, and earliest-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
      if (!play_issue_2265_arven_quick_ball_surplus_route()) {
        throw std::logic_error("Issue-2265 Arven Quick Ball route became unavailable");
      }
      return;
    }
    if (play_issue_2225_arven_vessel_finish()) return;
    choose_supporter_issue2225_original();
  }

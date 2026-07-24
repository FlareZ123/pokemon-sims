  bool issue_1419_oricorio_continuation_ = false;

  bool issue_1419_post_search_continuation_available() const {
    const int missing_attachments = grass_needed() + fire_needed();
    int searchable_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      searchable_payloads += deck_count_after_search_started(card);
    }

    // Quick Ball has established K1. Continue reserving the route only when the
    // now-known deck still contains every needed Basic Energy and at least one
    // Dragon payload after the worst-case intervening draws:
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // K1 and multi-prize collapse policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1419
    return deck_seen_ && active_is_vstar() && missing_attachments > 0 &&
        state_.turn + missing_attachments <= scenario_.max_turn &&
        deck_count_after_search_started(Card::Grass) >= grass_needed() &&
        deck_count_after_search_started(Card::Fire) >= fire_needed() &&
        searchable_payloads > missing_attachments &&
        hand_count(Card::BrilliantBlender) > 0;
  }

  bool issue_1419_should_hold_supporter() const {
    return issue_1419_oricorio_continuation_ && active_is_vstar() &&
        hand_count(Card::BrilliantBlender) > 0 &&
        (need_energy() || need_payload());
  }

  bool issue_1419_play_quick_ball_route() {
    const auto route_cost = quick_ball_latias_oricorio_route_cost();
    if (route_cost != Card::LatiasEx) return false;

    // Quick Ball pays its other-card cost before searching. After that first legal
    // deck inspection establishes K1, prefer Oricorio when it is present. If the
    // singleton is Prized, complete the legal search with the ordinary K1 fallback
    // rather than using hidden Prize information to suppress the K0 action:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official Item, search, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1419
    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(Card::LatiasEx, "Quick Ball cost",
                           "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1419 Latias ex cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball");

    Card target = Card::Oricorio;
    bool found = move_deck_to_hand(target);
    if (!found) {
      target = fallback_quick_ball_target_after_search_started();
      found = move_deck_to_hand(target);
    }
    shuffle(state_.deck);
    issue_1419_oricorio_continuation_ =
        found && target == Card::Oricorio &&
        issue_1419_post_search_continuation_available();
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          found ? "Searched a Basic Pokémon: " + std::string(name(target)) + "."
                : "No Basic Pokémon remained after search.");
    return true;
  }

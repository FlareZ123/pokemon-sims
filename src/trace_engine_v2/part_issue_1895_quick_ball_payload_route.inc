  std::optional<Card> issue_1895_quick_ball_payload_cost() const {
    // Preserve the established one-cost Quick Ball plus prior-turn Forretress route.
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Confirmed route specification: https://github.com/FlareZ123/pokemon-sims/issues/1744
    const bool complete_public_route =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        !issue_1744_quick_ball_forretress_route_available() &&
        supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);
    if (!complete_public_route) return std::nullopt;

    // Quick Ball may discard any other card from hand. In this public K1 GF
    // state, held Crispin attaches the missing Grass, so a held Dragon is DCI 1
    // because Quick Ball's printed cost establishes the current-turn Apex Dragon
    // payload while its legal Basic target may remain in hand:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supplied card-data corpus: https://github.com/PokemonTCG/pokemon-tcg-data/tree/master/cards/en
    // Official Item, discard-cost, search, Supporter, Energy-attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // K1, dynamic DCI/JIT, Supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1895
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1895_held_crispin_quick_ball_route_available() const {
    return issue_1895_quick_ball_payload_cost().has_value();
  }

  bool complete_issue_1895_held_crispin_quick_ball_route() {
    const std::optional<Card> cost = issue_1895_quick_ball_payload_cost();
    if (!cost.has_value()) return false;

    // Execute the confirmed complete route before generic Quick Ball selection can
    // preserve the high-DCI Dragon. The Basic search is mandatory, remains legal,
    // and does not consume Bench space because the searched card may stay in hand:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Official Item, discard-cost, search, shuffle, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1895
    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(*cost, "Quick Ball issue-1895 payload cost",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1895 Quick Ball payload cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1895 payload route");

    Card target = fallback_quick_ball_target_after_search_started();
    if (!is_basic(target) || deck_count_after_search_started(target) == 0) {
      const auto first_basic = std::find_if(
          state_.deck.begin(), state_.deck.end(), is_basic);
      if (first_basic == state_.deck.end()) {
        throw std::logic_error("Issue-1895 legal Basic target disappeared");
      }
      target = *first_basic;
    }
    if (!move_deck_to_hand(target)) {
      throw std::logic_error("Issue-1895 Quick Ball target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-JIT-01",
          "Quick Ball discarded the held Dragon payload and searched " +
              std::string(name(target)) + ".");

    if (!play_crispin()) {
      throw std::logic_error("Issue-1895 Crispin finish disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

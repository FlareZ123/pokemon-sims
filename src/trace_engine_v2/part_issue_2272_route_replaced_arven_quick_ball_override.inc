  bool issue_2272_route_replaced_arven_quick_ball_available() const {
    // At K1, every remaining physical target is known. When Quick Ball is already
    // held, Arven's Item-search role is replaced for this exact completion: Quick
    // Ball can search Latias ex, Skyliner can give the Basic Active free Retreat,
    // and the already-held Brilliant Blender can independently put a Dragon payload
    // into discard during the same strict-JIT turn. Arven may therefore become the
    // one-card Quick Ball cost only after the VSTAR and GGF Energy axes are already
    // complete and every other completion gate is observable.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, search, Bench, Ability, Retreat, and Supporter procedure:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 / strict-JIT / dynamic DCI / earliest-route policy:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2272
    if (scenario_.dci != DciProfile::StrictJit || !prizes_known() || item_locked() ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::Arven) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || need_regi() || need_vstar() ||
        need_energy() || !need_active_vstar() || !need_payload() ||
        !benched_vstar_promotion_ready() || !state_.active ||
        !is_basic(state_.active->card) || state_.retreat_used || bench_space() <= 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload) ||
        !can_play_payload_this_turn() ||
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Keep every existing lower-DCI Quick Ball cost ahead of Arven. In particular,
    // a post-turn-one Battle VIP Pass or another already-discardable card must be
    // spent before this live Supporter. Existing Latias-specific surplus-Energy and
    // replaced-Tate fallbacks likewise retain their prior ordering.
    // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2272
    return !choose_discard(false, true, true, Card::QuickBall).has_value() &&
           !quick_ball_final_surplus_energy_cost().has_value() &&
           !quick_ball_latias_replaced_tate_cost().has_value();
  }

  bool play_issue_2272_route_replaced_arven_quick_ball() {
    if (!issue_2272_route_replaced_arven_quick_ball_available()) return false;

    // The printed Quick Ball cost is paid before the Basic search. K1 proves Latias
    // ex remains in deck, while the held Blender preserves a separate same-turn
    // payload channel. This branch is deliberately reached before generic Quick Ball
    // handling so the otherwise protected Arven is spent only in this complete route.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Trainer resolution ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // Dynamic DCI and route priority:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2272
    if (!remove_one(state_.hand, Card::QuickBall)) {
      throw std::logic_error("issue-2272 Quick Ball disappeared");
    }
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(Card::Arven, "route-replaced Quick Ball cost",
                           "R-QB-01; P-DCI-01")) {
      throw std::logic_error("issue-2272 Arven cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-2272 route");
    if (!move_deck_to_hand(Card::LatiasEx)) {
      throw std::logic_error("issue-2272 Latias ex target disappeared");
    }
    shuffle(state_.deck);
    trace("QUICK BALL", "R-QB-01; R-GAME-ITEM; P-DCI-01; P-CONNECTOR-01",
          "Discarded route-replaced Arven; searched Latias ex for the Blender-backed Skyliner finish.");
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (play_issue_2272_route_replaced_arven_quick_ball()) return true;
    return play_quick_ball_issue2272_original(permit_payload);
  }

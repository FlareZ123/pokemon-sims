  std::optional<Card> issue_2269_treasure_latias_blender_cost() const {
    const Pokemon* target = target_regi();
    if (!strict_payload_timing() || item_locked() || !prizes_known() ||
        target == nullptr || target->card != Card::RegidragoVstar ||
        // Apex readiness is semantic so DDE + one Basic is accepted exactly like
        // Basic GGF rather than recreating a raw Grass/Fire migration hole.
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // DDE semantic-energy contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
        !pays_apex_energy_cost(*target) || !need_active_vstar() ||
        !need_payload() || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) ||
        // The established Tapu-active scheduler remains authoritative. This route
        // covers the distinct Oricorio/other-Basic Active state from #2269.
        // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
        // Oricorio GRI 55: https://api.pokemontcg.io/v2/cards/sm2-55
        // Existing Tapu scheduler scope: https://github.com/FlareZ123/pokemon-sims/issues/1533
        // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2269
        state_.active->card == Card::TapuLeleGX || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        std::none_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return std::nullopt;
    }

    // The selected attacker already pays Apex. Brilliant Blender independently
    // supplies the strict current-turn Dragon payload from the K1-known deck, so
    // ordinary setup-surplus cards may pay Treasure. Existing DCI selectors keep
    // precedence before the explicit Energy fallbacks.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed stale-claim regression: https://github.com/FlareZ123/pokemon-sims/issues/2269
    auto cost = choose_discard(false, true, true, Card::MysteriousTreasure);
    if (!cost) cost = quick_ball_final_surplus_energy_cost();
    if (!cost && hand_count(Card::Grass) > 0) cost = Card::Grass;
    if (!cost && hand_count(Card::Fire) > 0) cost = Card::Fire;
    return cost;
  }

  bool issue_2269_play_treasure_latias_blender_route() {
    const auto cost = issue_2269_treasure_latias_blender_cost();
    if (!cost) return false;

    // Treasure finds Latias ex after paying the route-proven surplus cost. Blender
    // establishes the same-turn Dragon payload, then Skyliner gives the Basic
    // Active no Retreat Cost into the semantically powered Benched VSTAR.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, search, discard, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2269
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*cost,
                           "Mysterious Treasure issue-2269 Latias route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-2269 Treasure cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-2269 Latias route");
    if (!move_deck_to_hand(Card::LatiasEx)) {
      throw std::logic_error("Issue-2269 Latias ex target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          "Spent the route-proven surplus cost and searched Latias ex for the Basic-Active promotion route.");
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("Issue-2269 Latias ex Bench action failed");
    }
    if (!play_brilliant_blender()) {
      throw std::logic_error("Issue-2269 Brilliant Blender payload disappeared");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("Issue-2269 Skyliner promotion disappeared");
    }
    return true;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    // Preserve the merged #2265 continuation while its resource hold is live, then
    // give every established Treasure route first refusal. The #2269 route is a
    // fallback for the distinct Basic-Active/Blender state that existing policy
    // cannot complete. This keeps earlier route contracts and avoids one-off
    // preemption when two legal Latias routes overlap.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Refined confirmed #2265 route: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
    // Confirmed #2269 fallback scope: https://github.com/FlareZ123/pokemon-sims/issues/2269
    // Repository earliest-complete-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_2265_observable_direct_route_should_hold_treasure() ||
        issue_2265_hold_treasure_for_direct_route()) {
      return false;
    }
    if (play_mysterious_treasure_issue2269_original(permit_payload)) return true;
    return issue_2269_play_treasure_latias_blender_route();
  }

  bool play_gladion() {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0) return false;

    if (need_vstar() && hand_count(Card::RegidragoVstar) == 0) {
      // Mysterious Treasure needs one other card from hand. Ultra Ball needs two
      // other cards. Only a payable search route may preserve Gladion instead of
      // recovering a prized VSTAR: https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/sm4-95
      const bool playable_mysterious_vstar_search = !item_locked() &&
          hand_count(Card::MysteriousTreasure) > 0 &&
          is_legal_cost_available(can_play_payload_this_turn(), true);
      const auto playable_ultra_vstar_search = [this]() {
        if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
        const std::vector<Card> original_hand = state_.hand;
        const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
        const bool permit_payload = can_play_payload_this_turn();
        const auto first = choose_discard(permit_payload, true);
        if (!first || !remove_one(state_.hand, *first)) {
          restore_hand();
          return false;
        }
        const auto second = choose_discard(permit_payload && can_play_payload_this_turn() && !is_payload(*first), true);
        restore_hand();
        return second.has_value();
      };
      const bool playable_fss_vstar_route = !state_.vstar_power_used &&
          hand_count(Card::ForestSealStone) > 0 &&
          ((state_.active && state_.active->card == Card::RegidragoV && state_.active->tool == Tool::None) ||
           std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) {
             return p.card == Card::RegidragoV && p.tool == Tool::None;
           }));
      // Arven remains a Supporter under Item lock and may fetch the independently
      // playable Forest Seal Stone Tool route: https://api.pokemontcg.io/v2/cards/sv1-166
      // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
      const bool playable_arven_fss_vstar_route = !state_.vstar_power_used &&
          hand_count(Card::Arven) > 0 && might_be_unseen(Card::ForestSealStone) &&
          ((state_.active && state_.active->card == Card::RegidragoV && state_.active->tool == Tool::None) ||
           std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) {
             return p.card == Card::RegidragoV && p.tool == Tool::None;
           }));
      const bool live_vstar_route = hand_count(Card::EvolutionIncense) > 0 ||
          playable_mysterious_vstar_search || playable_ultra_vstar_search() ||
          playable_fss_vstar_route || playable_arven_fss_vstar_route;
      if (live_vstar_route) return false;

      // The original route classifier predates the payable-cost distinction and
      // treats a held Mysterious Treasure or Ultra Ball as live by identity alone.
      // Hide only those proven-unpayable cards while it determines whether Gladion
      // is critical, then restore the exact card multiset after the policy decision.
      std::vector<Card> held_unpayable_routes;
      while (remove_one(state_.hand, Card::MysteriousTreasure)) held_unpayable_routes.push_back(Card::MysteriousTreasure);
      while (remove_one(state_.hand, Card::UltraBall)) held_unpayable_routes.push_back(Card::UltraBall);
      const bool played = play_gladion_original();
      state_.hand.insert(state_.hand.end(), held_unpayable_routes.begin(), held_unpayable_routes.end());
      return played;
    }

    return play_gladion_original();
  }

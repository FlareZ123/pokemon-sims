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
      // Evolution Incense is an Item that searches the deck for an Evolution Pokémon:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // Item-lock effects prevent Item cards from being played from hand:
      // https://api.pokemontcg.io/v2/cards/me2pt5-16
      const bool playable_evolution_incense_vstar_route = !item_locked() &&
          hand_count(Card::EvolutionIncense) > 0 && might_be_unseen(Card::RegidragoVstar);
      // Gladion can immediately exchange itself for a known prized VSTAR:
      // https://api.pokemontcg.io/v2/cards/sm4-95
      // Star Alchemy searches the deck, so Arven into Forest Seal Stone cannot
      // recover that VSTAR this turn after K1 proves it is absent from deck:
      // https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv1-166
      // A player may play only one Supporter each turn, so that delayed route cannot
      // use Gladion until the next turn: https://www.pokemon.com/us/pokemon-tcg/rules
      const bool known_prized_vstar_absent_from_deck = prizes_known() &&
          prize_count_after_reveal(Card::RegidragoVstar) > 0 &&
          deck_count_after_search_started(Card::RegidragoVstar) == 0;
      const bool playable_arven_fss_vstar_route = !known_prized_vstar_absent_from_deck &&
          !state_.vstar_power_used && hand_count(Card::Arven) > 0 && might_be_unseen(Card::ForestSealStone) &&
          ((state_.active && state_.active->card == Card::RegidragoV && state_.active->tool == Tool::None) ||
           std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) {
             return p.card == Card::RegidragoV && p.tool == Tool::None;
           }));
      const bool live_vstar_route = playable_evolution_incense_vstar_route ||
          playable_mysterious_vstar_search || playable_ultra_vstar_search() ||
          playable_fss_vstar_route || playable_arven_fss_vstar_route;
      if (live_vstar_route) return false;

      // The original route classifier predates the playable-route distinction and
      // treats a held Mysterious Treasure, Ultra Ball, or Evolution Incense as live
      // by identity alone. Hide only proven-unpayable or unavailable cards while it
      // determines whether Gladion is critical, then restore the exact card multiset.
      std::vector<Card> held_unpayable_routes;
      while (remove_one(state_.hand, Card::MysteriousTreasure)) held_unpayable_routes.push_back(Card::MysteriousTreasure);
      while (remove_one(state_.hand, Card::UltraBall)) held_unpayable_routes.push_back(Card::UltraBall);
      if (!playable_evolution_incense_vstar_route) {
        while (remove_one(state_.hand, Card::EvolutionIncense)) held_unpayable_routes.push_back(Card::EvolutionIncense);
      }
      if (known_prized_vstar_absent_from_deck) {
        while (remove_one(state_.hand, Card::Arven)) held_unpayable_routes.push_back(Card::Arven);
      }
      const bool played = play_gladion_original();
      state_.hand.insert(state_.hand.end(), held_unpayable_routes.begin(), held_unpayable_routes.end());
      return played;
    }

    return play_gladion_original();
  }

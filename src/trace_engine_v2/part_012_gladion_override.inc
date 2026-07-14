  bool play_gladion() {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0) return false;

    const auto playable_ultra_search = [this]() {
      if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      const bool permit_payload = can_play_payload_this_turn();
      // Ultra Ball requires two other cards. Keep the played copy in hand but
      // exclude it from both cost selections, allowing a distinct second copy:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      const auto first = choose_discard(permit_payload, true, true, Card::UltraBall);
      if (!first || !remove_one(state_.hand, *first)) {
        restore_hand();
        return false;
      }
      const auto second = choose_discard(permit_payload && !is_payload(*first),
                                         true, true, Card::UltraBall);
      restore_hand();
      return second.has_value();
    };

    if (need_regi() && hand_count(Card::RegidragoV) == 0 && hand_count(Card::UltraBall) > 0 &&
        !playable_ultra_search()) {
      // Gladion can recover the missing Basic from Prizes. An Ultra Ball that
      // cannot pay its two-card cost is not a competing route:
      // https://api.pokemontcg.io/v2/cards/sm4-95
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      std::vector<Card> held_unpayable_ultra_balls;
      while (remove_one(state_.hand, Card::UltraBall)) held_unpayable_ultra_balls.push_back(Card::UltraBall);
      const bool played = play_gladion_original();
      state_.hand.insert(state_.hand.end(), held_unpayable_ultra_balls.begin(), held_unpayable_ultra_balls.end());
      return played;
    }

    if (need_vstar() && hand_count(Card::RegidragoVstar) == 0) {
      // Mysterious Treasure needs one other card from hand. Reserve the copy being
      // played so a distinct Ultra Ball remains a legal one-card discard cost:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // Only a payable search route may preserve Gladion instead of recovering a
      // prized VSTAR: https://api.pokemontcg.io/v2/cards/sm4-95
      const bool playable_mysterious_vstar_search = !item_locked() &&
          hand_count(Card::MysteriousTreasure) > 0 &&
          choose_discard(can_play_payload_this_turn(), true, true,
                         Card::MysteriousTreasure).has_value();
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
      // Policy prioritizes immediate known-prize recovery over slower connectors:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Gladion can immediately exchange itself for a known prized VSTAR:
      // https://api.pokemontcg.io/v2/cards/sm4-95
      // Star Alchemy searches the deck, so Arven into Forest Seal Stone cannot
      // recover that VSTAR this turn after K1 proves it is absent from deck:
      // https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv1-166
      // A player may play only one Supporter each turn, so that delayed route cannot
      // use Gladion until the next turn: https://www.pokemon.com/us/pokemon-tcg/rules
      // K1 certainty makes this a direct recovery choice, not a speculative route.
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
          playable_mysterious_vstar_search || playable_ultra_search() ||
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

    // Gladion can recover a known prized Dragon payload when a one-discard Item
    // remains in hand to put that payload into discard during the same turn:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool live_one_discard_payload_outlet = !item_locked() && !state_.deck.empty() &&
        (hand_count(Card::EarthenVessel) > 0 || hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0);
    if (prizes_known() && need_payload() && can_play_payload_this_turn() &&
        live_one_discard_payload_outlet) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX}) {
        if (prize_count_after_reveal(payload) == 0) continue;
        remove_one(state_.hand, Card::Gladion);
        state_.supporter_used = true;
        state_.prizes_revealed = true;
        const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), payload);
        *it = Card::Gladion;
        state_.hand.push_back(payload);
        shuffle(state_.prizes);
        trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-DCI-01",
              "Looked at Prize cards and exchanged Gladion for " + std::string(name(payload)) +
                  " to use a same-turn hand-discard outlet.");
        return true;
      }
    }

    return play_gladion_original();
  }

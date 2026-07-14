  bool has_payable_payload_outlet_after_costed_search(const Card excluded) {
    // Mysterious Treasure and Quick Ball discard from hand before searching, while
    // Ultra Ball must later discard the fetched payload plus one other hand card:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Strict DCI protects recovery and singleton cards until a route can actually
    // complete the current-turn payload axis:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    if (!payload_might_be_in_deck()) return false;
    // The Item being modeled cannot pay its own cost. A distinct held Ultra Ball
    // remains a legal cost for either Mysterious Treasure or Quick Ball even when
    // Ultra Ball cannot satisfy its own two-card play cost:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const auto current_cost = choose_discard(false, true, true, excluded);
    if (!current_cost) return false;

    const auto outlet_survives_current_search = [this, excluded, current_cost](const Card outlet) {
      const int played_copy = outlet == excluded ? 1 : 0;
      const int cost_copy = *current_cost == outlet ? 1 : 0;
      return hand_count(outlet) > played_copy + cost_copy;
    };

    const auto ultra_ball_can_pay_after_current_search = [this, excluded, current_cost]() {
      if (hand_count(Card::UltraBall) == 0) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      if (!remove_one(state_.hand, excluded) || !remove_one(state_.hand, *current_cost)) {
        restore_hand();
        return false;
      }

      // A fetched Dragon represents the searched payload. Ultra Ball may use it as
      // one cost only if a distinct policy-legal second cost still remains:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      state_.hand.push_back(Card::MegaDragonite);
      const auto payload_cost = choose_discard(true, true);
      if (!payload_cost || !is_payload(*payload_cost) || !remove_one(state_.hand, *payload_cost)) {
        restore_hand();
        return false;
      }
      // Reserve one surviving Ultra Ball as the card being played. A distinct
      // surviving Ultra Ball may legally be its second discard cost:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      const bool payable = choose_discard(false, true, true, Card::UltraBall).has_value();
      restore_hand();
      return payable;
    };

    // The initiating search removes the fetched payload. Every later search Item
    // needs at least one remaining deck card or it is known to have no effect and
    // cannot be played. Serena stays live because discarding the payload is itself
    // an effect even when the deck is then empty:
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    const bool later_search_can_begin = state_.deck.size() > 1U;
    return (later_search_can_begin &&
            (outlet_survives_current_search(Card::MysteriousTreasure) ||
             outlet_survives_current_search(Card::QuickBall) ||
             outlet_survives_current_search(Card::EarthenVessel) ||
             ultra_ball_can_pay_after_current_search())) ||
           (supporter_allowed() && outlet_survives_current_search(Card::Serena));
  }

  bool play_arven() {
    // Arven is a Supporter, so Item lock does not prohibit playing it: https://www.pokemon.com/us/pokemon-tcg/rules
    // Pokémon Tools are separate from Item cards after the Tool erratum, leaving Forest Seal Stone playable through Item lock: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    if (!supporter_allowed() || hand_count(Card::Arven) == 0) return false;

    // Pokémon V includes Pokémon VSTAR, so Arven may find Forest Seal Stone for an
    // open Regidrago V or Regidrago VSTAR Tool slot:
    // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    // https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/swsh12-156
    const bool needs_tool = !state_.vstar_power_used &&
                            ((state_.active && is_pokemon_v(state_.active->card) && state_.active->tool == Tool::None) ||
                             std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) {
                               return is_pokemon_v(p.card) && p.tool == Tool::None;
                             }));
    const bool can_pay_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);

    const auto item_candidates_for_current_knowledge = [this, can_pay_search_cost]() {
      const bool live_regi_target = hand_count(Card::RegidragoV) == 0 && might_be_unseen(Card::RegidragoV);
      const bool live_vstar_target = hand_count(Card::RegidragoVstar) == 0 && might_be_unseen(Card::RegidragoVstar);
      const bool live_energy_target = (grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
                                      (fire_needed() > 0 && might_be_unseen(Card::Fire));
      const bool live_latias_target = hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx) &&
                                      might_be_unseen(Card::LatiasEx);
      const bool live_payload_target = might_be_unseen(Card::MegaDragonite) ||
                                       might_be_unseen(Card::Dragapult) ||
                                       might_be_unseen(Card::GoodraVstar) ||
                                       might_be_unseen(Card::DialgaGX);
      const bool known_prized_regi_target = prizes_known() && need_regi() && bench_space() > 0 &&
                                             prize_count_after_reveal(Card::RegidragoV) > 0;
      std::vector<Card> candidates;
      // Item lock prevents Arven's Item target from advancing this turn; its Tool target remains independent: https://api.pokemontcg.io/v2/cards/sv1-166 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
      if (item_locked()) return candidates;
      const auto add_item_candidate = [this, &candidates](const Card item, const bool advances_now) {
        if (advances_now && might_be_unseen(item) &&
            std::find(candidates.begin(), candidates.end(), item) == candidates.end()) {
          candidates.push_back(item);
        }
      };

      if (need_regi() && bench_space() > 0 && live_regi_target) {
        add_item_candidate(Card::QuickBall, can_pay_search_cost);
        add_item_candidate(Card::MysteriousTreasure, can_pay_search_cost);
      }
      if (known_prized_regi_target) add_item_candidate(Card::HisuianHeavyBall, true);
      if (need_vstar() && live_vstar_target) {
        add_item_candidate(Card::EvolutionIncense, true);
        add_item_candidate(Card::MysteriousTreasure, can_pay_search_cost);
      }
      if (need_energy() && live_energy_target) add_item_candidate(Card::EarthenVessel, can_pay_search_cost);
      if (need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) && live_latias_target) {
        add_item_candidate(Card::QuickBall, can_pay_search_cost);
        add_item_candidate(Card::MysteriousTreasure, can_pay_search_cost);
      }
      if (can_play_payload_this_turn() && live_payload_target) add_item_candidate(Card::BrilliantBlender, true);
      return candidates;
    };

    const auto fss_has_live_target = [this]() {
      // Star Alchemy may search Gladion after K1 proves the required VSTAR is prized; Gladion recovers that Prize card on a later turn: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sm4-95
      const bool known_prized_vstar_gladion_route = prizes_known() && need_vstar() &&
          prize_count_after_reveal(Card::RegidragoVstar) > 0 &&
          deck_count_after_search_started(Card::Gladion) > 0;
      return known_prized_vstar_gladion_route ||
             (need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV)) ||
             (need_vstar() && might_be_unseen(Card::RegidragoVstar)) ||
             (need_energy() && ((grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
                                (fire_needed() > 0 && might_be_unseen(Card::Fire)))) ||
             (need_active_vstar() && bench_space() > 0 && !in_play(Card::LatiasEx) &&
              might_be_unseen(Card::LatiasEx)) ||
             (can_play_payload_this_turn() && (might_be_unseen(Card::MegaDragonite) ||
                                               might_be_unseen(Card::Dragapult) ||
                                               might_be_unseen(Card::GoodraVstar) ||
                                               might_be_unseen(Card::DialgaGX)));
    };

    const auto powerglass_has_live_target = [this]() {
      if (!state_.active || state_.active->tool != Tool::None ||
          (state_.active->card != Card::RegidragoV && state_.active->card != Card::RegidragoVstar)) {
        return false;
      }
      // Powerglass is a Pokémon Tool that may attach a Basic Energy from discard to
      // its Active holder at end of turn. Arven may select it after legal inspection
      // proves the higher-priority Forest Seal Stone absent:
      // https://api.pokemontcg.io/v2/cards/sv1-166
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sv6pt5-63
      const bool needed_grass_in_discard = state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0;
      const bool needed_fire_in_discard = state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0;
      return (needed_grass_in_discard || needed_fire_in_discard) && might_be_unseen(Card::Powerglass);
    };

    const std::vector<Card> pre_search_candidates = item_candidates_for_current_knowledge();
    const bool pre_search_live_fss = needs_tool && fss_has_live_target() && might_be_unseen(Card::ForestSealStone);
    const bool pre_search_live_powerglass = powerglass_has_live_target();
    if (pre_search_candidates.empty() && !pre_search_live_fss && !pre_search_live_powerglass) return false;

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;

    record_deck_search_knowledge("Arven");
    const std::vector<Card> item_candidates = item_candidates_for_current_knowledge();
    const bool live_fss = needs_tool && fss_has_live_target() && might_be_unseen(Card::ForestSealStone);
    const bool live_powerglass = powerglass_has_live_target();
    std::vector<Card> found;
    for (const Card item : item_candidates) {
      if (move_deck_to_hand(item)) { found.push_back(item); break; }
    }
    if (live_fss && move_deck_to_hand(Card::ForestSealStone)) {
      found.push_back(Card::ForestSealStone);
    } else if (live_powerglass && move_deck_to_hand(Card::Powerglass)) {
      found.push_back(Card::Powerglass);
    }
    shuffle(state_.deck);
    const bool selected_powerglass = std::find(found.begin(), found.end(), Card::Powerglass) != found.end();
    trace("PLAY SUPPORTER", selected_powerglass ? "R-ARVEN-01; R-GAME-SUPPORTER; R-POWERGLASS-01"
                                                : "R-ARVEN-01; R-GAME-SUPPORTER",
          "Searched an Item and a Pokémon Tool: " + join_cards(found) + ".");
    return true;
  }

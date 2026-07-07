  bool play_arven() {
    // Arven is a Supporter, so Item lock does not prohibit playing it: https://www.pokemon.com/us/pokemon-tcg/rules
    // Pokémon Tools are separate from Item cards after the Tool erratum, leaving Forest Seal Stone playable through Item lock: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    if (!supporter_allowed() || hand_count(Card::Arven) == 0) return false;

    const bool needs_tool = has_regi_v() && !state_.vstar_power_used &&
                            ((state_.active && state_.active->card == Card::RegidragoV && state_.active->tool == Tool::None) ||
                             std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoV && p.tool == Tool::None; }));
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

    const std::vector<Card> pre_search_candidates = item_candidates_for_current_knowledge();
    const bool pre_search_live_tool = needs_tool && fss_has_live_target() && might_be_unseen(Card::ForestSealStone);
    if (pre_search_candidates.empty() && !pre_search_live_tool) return false;

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;

    record_deck_search_knowledge("Arven");
    const std::vector<Card> item_candidates = item_candidates_for_current_knowledge();
    const bool live_tool = needs_tool && fss_has_live_target() && might_be_unseen(Card::ForestSealStone);
    std::vector<Card> found;
    for (const Card item : item_candidates) {
      if (move_deck_to_hand(item)) { found.push_back(item); break; }
    }
    if (live_tool && move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER", "Searched an Item and a Pokémon Tool: " + join_cards(found) + ".");
    return true;
  }

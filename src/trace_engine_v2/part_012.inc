      if (move_deck_to_hand(target)) found.push_back(target);
    }
    for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, payload_outlet,
                                Card::MysteriousTreasure, Card::EarthenVessel, Card::ForestSealStone,
                                Card::LatiasEx, Card::QuickBall}) {
      if (found.size() >= 3U) break;
      // Crispin needs different Basic Energy types; K1 must not fetch it after the inspected deck proves no needed Energy remains: https://api.pokemontcg.io/v2/cards/sv7-133
      if (fallback == Card::Crispin && !crispin_can_advance_energy_axis()) continue;
      if (move_deck_to_hand(fallback)) found.push_back(fallback);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER", "Searched up to 3 cards: " + join_cards(found) + ". Turn ended.");
    return true;
  }

  bool play_arven() {
    if (!supporter_allowed() || item_locked() || hand_count(Card::Arven) == 0) return false;

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
      std::vector<Card> candidates;
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
      return (need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV)) ||
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

    // Arven searches an Item and a Tool: https://api.pokemontcg.io/v2/cards/sv1-166
    // At K0, play is justified by a plausible current-axis route. Once the deck
    // is legally visible, targets must be selected from the K1 candidate list.
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

  bool play_gladion() {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0) return false;
    // Quick Ball and Mysterious Treasure require their hand-discard costs before their
    // Regidrago V search routes exist: https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sm6-113
    const bool playable_one_discard_regi_search = !item_locked() &&
        is_legal_cost_available(can_play_payload_this_turn(), true) &&
        (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0);
    const bool no_regi_route = need_regi() && hand_count(Card::RegidragoV) == 0 &&
        !playable_one_discard_regi_search && hand_count(Card::UltraBall) == 0 &&
        (item_locked() || hand_count(Card::Arven) == 0);
    const bool no_vstar_route = need_vstar() && hand_count(Card::RegidragoVstar) == 0 && hand_count(Card::EvolutionIncense) == 0 && hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::UltraBall) == 0 && (item_locked() || hand_count(Card::Arven) == 0) && hand_count(Card::ForestSealStone) == 0;
    const bool no_locked_payload_route = need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) == 0;
    const bool blind_critical = no_regi_route || no_vstar_route || no_locked_payload_route;

    std::optional<Card> known_target;
    if (prizes_known()) {
      // Card text source: https://api.pokemontcg.io/v2/cards/sm4-95
      // A known prized Regidrago or VSTAR is an immediate missing-card axis.
      // These exact targets beat slower Supporter connectors.
      if (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) known_target = Card::RegidragoVstar;
      else if (need_regi() && bench_space() > 0 && prize_count_after_reveal(Card::RegidragoV) > 0) known_target = Card::RegidragoV;
      else {
        const bool live_crispin = need_energy() && hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis();
        const bool live_burnet = need_payload() && hand_count(Card::ProfessorBurnet) > 0 && can_play_payload_this_turn();
        const bool live_tate_switch = need_active_vstar() && !need_energy() && !need_payload() && hand_count(Card::TateLiza) > 0;
        const auto prize_energy = final_prized_energy_for_gladion();
        const bool same_energy_absent_from_deck = prize_energy.has_value() &&
            deck_count_after_search_started(*prize_energy) == 0;

        if (prize_energy && same_energy_absent_from_deck && !live_crispin) {
          known_target = *prize_energy;
        } else if (need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) && !live_tate_switch &&
                   prize_count_after_reveal(Card::LatiasEx) > 0) {
          known_target = Card::LatiasEx;
        } else if (need_payload() && !live_burnet && !item_locked() && payload_window_open() &&
                   prize_count_after_reveal(Card::BrilliantBlender) > 0) {
          known_target = Card::BrilliantBlender;
        } else if (need_energy() && !live_crispin && prize_count_after_reveal(Card::Oricorio) > 0 &&
                   bench_space() > 0 && !in_play(Card::Oricorio) && needs_oricorio_connector()) {
          known_target = Card::Oricorio;
        } else if (need_energy() && !live_crispin && !item_locked() && prize_count_after_reveal(Card::EarthenVessel) > 0 &&
                   is_legal_cost_available(can_play_payload_this_turn(), true)) {
          known_target = Card::EarthenVessel;
        }
      }

      const bool direct_supporter = should_play_steven() ||
                                    (need_energy() && hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis()) ||
                                    (need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) > 0) ||
                                    (!item_locked() && hand_count(Card::Arven) > 0);
      if (!known_target && !direct_supporter && prize_count_after_reveal(Card::Lusamine) > 0 &&
          (count_of(state_.discard, Card::Crispin) > 0 || count_of(state_.discard, Card::ProfessorBurnet) > 0 ||
           count_of(state_.discard, Card::Arven) > 0 || count_of(state_.discard, Card::StevensResolve) > 0)) {
        known_target = Card::Lusamine;
      }
    }
    if (!known_target && !blind_critical) return false;

    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto exchange = [this](const Card target) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), target);
      if (it == state_.prizes.end()) return false;
      const Card chosen = *it;
      *it = Card::Gladion;
      state_.hand.push_back(chosen);
      shuffle(state_.prizes);
      trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01", "Looked at Prize cards and exchanged Gladion for " + std::string(name(chosen)) + ".");
      return true;
    };
    if (known_target && exchange(*known_target)) return true;
    for (const Card target : {Card::RegidragoVstar, Card::RegidragoV, Card::ProfessorBurnet, Card::Crispin, Card::MegaDragonite}) {
      if (exchange(target)) return true;
    }
    const std::array<Card, 8> lowest_impact{Card::Dipplin, Card::Grass, Card::Fire, Card::Powerglass, Card::FieldBlower, Card::ChaoticSwell, Card::PathToPeak, Card::MawileGX};
    auto selected = state_.prizes.begin();
    for (const Card low : lowest_impact) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), low);
      if (it != state_.prizes.end()) { selected = it; break; }
    }
    const Card chosen = *selected;
    *selected = Card::Gladion;
    state_.hand.push_back(chosen);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01", "Completed mandatory Prize exchange with " + std::string(name(chosen) + ".");
    return true;
  }

  bool play_serena() {
    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;
    const bool permit_payload = can_play_payload_this_turn() && std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const auto first = choose_discard(permit_payload, true);
    if (!first) return false;
    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    discard_from_hand(*first, "Serena chosen discard", "R-SERENA-01");
    if (scenario_.dci != DciProfile::StrictJit) {
      for (int number = 0; number < 2; ++number) {
        const auto extra = choose_discard(false, true);
        if (!extra || is_payload(*extra)) break;
        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::MawileGX || *extra == Card::Guzma ||
                               *extra == Card::Channeler || *extra == Card::FieldBlower || *extra == Card::ChaoticSwell || *extra == Card::PathToPeak;
        if (!can_cycle) break;

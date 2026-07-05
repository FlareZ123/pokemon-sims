      if (move_deck_to_hand(target)) found.push_back(target);
    }
    for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, payload_outlet,
                                Card::MysteriousTreasure, Card::EarthenVessel, Card::ForestSealStone,
                                Card::LatiasEx, Card::QuickBall}) {
      if (found.size() >= 3U) break;
      if (move_deck_to_hand(fallback)) found.push_back(fallback);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER", "Searched up to 3 cards: " + join_cards(found) + ". Turn ended.");
    return true;
  }

  bool play_arven() {
    if (!supporter_allowed() || item_locked() || hand_count(Card::Arven) == 0) return false;
    const bool needs_item = need_regi() || need_vstar() || need_energy() || can_play_payload_this_turn();
    const bool needs_tool = has_regi_v() && !state_.vstar_power_used &&
                            ((state_.active && state_.active->card == Card::RegidragoV && state_.active->tool == Tool::None) ||
                             std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoV && p.tool == Tool::None; }));
    if (!needs_item && !needs_tool) return false;
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;

    Card item = Card::MysteriousTreasure;
    if (need_regi()) item = Card::QuickBall;
    else if (need_vstar()) item = Card::EvolutionIncense;
    else if (need_energy()) item = Card::EarthenVessel;
    else if (can_play_payload_this_turn()) item = Card::BrilliantBlender;

    std::vector<Card> found;
    if (move_deck_to_hand(item)) found.push_back(item);
    if (needs_tool && move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER", "Searched an Item and a Pokémon Tool: " + join_cards(found) + ".");
    return true;
  }

  bool play_gladion() {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0) return false;
    const bool no_regi_route = need_regi() && hand_count(Card::RegidragoV) == 0 && hand_count(Card::QuickBall) == 0 && hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::UltraBall) == 0 && hand_count(Card::Arven) == 0;
    const bool no_vstar_route = need_vstar() && hand_count(Card::RegidragoVstar) == 0 && hand_count(Card::EvolutionIncense) == 0 && hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::UltraBall) == 0 && hand_count(Card::Arven) == 0 && hand_count(Card::ForestSealStone) == 0;
    const bool no_locked_payload_route = need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) == 0;
    const bool blind_critical = no_regi_route || no_vstar_route || no_locked_payload_route;

    std::optional<Card> known_target;
    if (prizes_known()) {
      // A known prized Regidrago or VSTAR is an immediate missing-card axis.
      // These exact targets beat slower Supporter connectors.
      if (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) known_target = Card::RegidragoVstar;
      else if (need_regi() && prize_count_after_reveal(Card::RegidragoV) > 0) known_target = Card::RegidragoV;
      else {
        const bool live_crispin = need_energy() && hand_count(Card::Crispin) > 0;
        const bool live_burnet = need_payload() && hand_count(Card::ProfessorBurnet) > 0 && can_play_payload_this_turn();
        const bool live_tate_switch = need_active_vstar() && !need_energy() && !need_payload() && hand_count(Card::TateLiza) > 0;

        if (need_active_vstar() && !live_tate_switch && prize_count_after_reveal(Card::LatiasEx) > 0) known_target = Card::LatiasEx;
        else if (need_payload() && !live_burnet && !item_locked() && payload_window_open() &&
                 prize_count_after_reveal(Card::BrilliantBlender) > 0) known_target = Card::BrilliantBlender;
        else if (need_energy() && !live_crispin && prize_count_after_reveal(Card::Oricorio) > 0 &&
                 bench_space() > 0 && !in_play(Card::Oricorio) && needs_oricorio_connector()) known_target = Card::Oricorio;
        else if (need_energy() && !live_crispin && !item_locked() && prize_count_after_reveal(Card::EarthenVessel) > 0 &&
                 is_legal_cost_available(can_play_payload_this_turn(), true)) known_target = Card::EarthenVessel;
      }

      const bool direct_supporter = should_play_steven() || (need_energy() && hand_count(Card::Crispin) > 0) ||
                                    (need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) > 0) ||
                                    hand_count(Card::Arven) > 0;
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
    trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01", "Completed mandatory Prize exchange with " + std::string(name(chosen)) + ".");
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

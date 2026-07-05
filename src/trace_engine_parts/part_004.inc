    for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_deck_to_discard(payload)) { discarded.push_back(payload); break; }
    }
    // Dipplin is an immediate DCI discard because the list has no Applin. It is a legal optional Blender target.
    if (move_deck_to_discard(Card::Dipplin)) discarded.push_back(Card::Dipplin);
    shuffle(state_.deck);
    outcome_.used_blender = true;
    trace("PLAY ITEM", "R-BLENDER-01; R-GAME-ITEM; P-DCI-01", "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

  bool attach_fss() {
    if (item_locked() || hand_count(Card::ForestSealStone) == 0) return false;
    Pokemon* target = nullptr;
    if (state_.active && is_pokemon_v(state_.active->card) && state_.active->tool == Tool::None) target = &*state_.active;
    if (target == nullptr) {
      for (Pokemon& p : state_.bench) {
        if (is_pokemon_v(p.card) && p.tool == Tool::None) { target = &p; break; }
      }
    }
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::ForestSealStone);
    target->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; R-GAME-ITEM", "Attached Forest Seal Stone to Regidrago V.");
    return true;
  }

  bool should_play_steven() const {
    if (state_.turn != 1 || scenario_.going_first || !has_regi_v()) return false;
    const int axes = static_cast<int>(need_vstar() && hand_count(Card::RegidragoVstar) == 0) +
                     static_cast<int>(need_energy() && hand_count(Card::Crispin) == 0) +
                     static_cast<int>(need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0);
    return axes >= 2;
  }

  Card fss_target_after_search_started() const {
    if (state_.turn == 1 && !scenario_.going_first && should_play_steven() && deck_count_after_search_started(Card::StevensResolve) > 0) return Card::StevensResolve;
    if (need_regi() && deck_count_after_search_started(Card::RegidragoV) > 0) return Card::RegidragoV;
    if (need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) > 0) return Card::RegidragoVstar;
    if (need_energy() && deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;
    if (need_payload() && !item_locked() && deck_count_after_search_started(Card::BrilliantBlender) > 0) return Card::BrilliantBlender;
    if (need_payload() && deck_count_after_search_started(Card::ProfessorBurnet) > 0) return Card::ProfessorBurnet;
    if (need_active_vstar() && !in_play(Card::LatiasEx) && deck_count_after_search_started(Card::LatiasEx) > 0) return Card::LatiasEx;
    if (deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    return Card::MysteriousTreasure;
  }

  bool use_fss() {
    if (state_.vstar_power_used) return false;
    Pokemon* holder = nullptr;
    if (state_.active && state_.active->tool == Tool::ForestSealStone) holder = &*state_.active;
    if (holder == nullptr) {
      for (Pokemon& p : state_.bench) if (p.tool == Tool::ForestSealStone) { holder = &p; break; }
    }
    if (holder == nullptr || !is_pokemon_v(holder->card)) return false;
    // R-FSS-02: activating a Tool's existing VSTAR Power is not playing an Item from hand.
    const bool useful = should_play_steven() || need_regi() || need_vstar() || need_energy() || need_payload() || need_active_vstar();
    if (!useful) return false;
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    const Card target = fss_target_after_search_started();
    const bool found = move_deck_to_hand(target);
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01", found ? "Searched any card: " + std::string(name(target)) + "." : "Selected target was not in deck.");
    return true;
  }

  bool play_crispin() {
    if (!supporter_allowed() || hand_count(Card::Crispin) == 0 || !need_energy() || target_regi() == nullptr) return false;
    // The play decision knows only that both basic Energy types are not exhausted from known zones.
    if (!might_be_unseen(Card::Grass) || !might_be_unseen(Card::Fire)) return false;
    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;

    const bool deck_has_grass = deck_count_after_search_started(Card::Grass) > 0;
    const bool deck_has_fire = deck_count_after_search_started(Card::Fire) > 0;
    if (!deck_has_grass || !deck_has_fire) {
      // The card may select up to two different types. With one type available, it can put that card in hand but cannot attach "the other".
      if (deck_has_grass) move_deck_to_hand(Card::Grass);
      else if (deck_has_fire) move_deck_to_hand(Card::Fire);
      shuffle(state_.deck);
      trace("PLAY SUPPORTER", "R-CRISPIN-01; R-GAME-SUPPORTER", "Only one Basic Energy type was searchable; put it into hand and attached none.");
      return true;
    }

    Pokemon* target = target_regi();
    const Card attach = grass_needed() > 0 ? Card::Grass : Card::Fire;
    const Card to_hand = attach == Card::Grass ? Card::Fire : Card::Grass;
    if (!remove_one(state_.deck, attach) || !move_deck_to_hand(to_hand)) {
      throw std::logic_error("Crispin target disappeared during legal search");
    }
    if (attach == Card::Grass) ++target->grass; else ++target->fire;
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-CRISPIN-01; R-GAME-SUPPORTER", "Searched " + std::string(name(attach)) + " and " + std::string(name(to_hand)) + "; attached " + std::string(name(attach)) + " to " + std::string(name(target->card)) + " and put " + std::string(name(to_hand)) + " into hand.");
    return true;
  }

  bool play_professor_burnet() {
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0 || !can_play_payload_this_turn()) return false;
    remove_one(state_.hand, Card::ProfessorBurnet);
    state_.discard.push_back(Card::ProfessorBurnet);
    state_.supporter_used = true;
    std::vector<Card> discarded;
    for (const Card card : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_deck_to_discard(card)) { discarded.push_back(card); break; }
    }
    if (move_deck_to_discard(Card::Dipplin)) discarded.push_back(Card::Dipplin);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-BURNET-01; R-GAME-SUPPORTER; P-DCI-01", "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !should_play_steven()) return false;
    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;

    const bool next_turn_item_locked = scenario_.locks == LockMode::FullItem || scenario_.locks == LockMode::FullCombined ||
                                       (scenario_.locks == LockMode::TurnTwoItem && state_.turn == 1);
    const Card payload_outlet = next_turn_item_locked ? Card::ProfessorBurnet : Card::BrilliantBlender;
    std::vector<Card> wanted;
    if (need_vstar() && hand_count(Card::RegidragoVstar) == 0) wanted.push_back(Card::RegidragoVstar);
    if (need_energy() && hand_count(Card::Crispin) == 0) wanted.push_back(Card::Crispin);
    if (need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0) wanted.push_back(payload_outlet);
    for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, payload_outlet, Card::MysteriousTreasure, Card::EarthenVessel, Card::LatiasEx}) {
      if (wanted.size() >= 3U) break;
      if (std::find(wanted.begin(), wanted.end(), fallback) == wanted.end()) wanted.push_back(fallback);
    }
    std::vector<Card> found;
    for (const Card target : wanted) if (move_deck_to_hand(target)) found.push_back(target);
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
    if (!no_regi_route && !no_vstar_route && !no_locked_payload_route) return false;

    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    std::vector<Card> priorities;
    if (no_regi_route) priorities.push_back(Card::RegidragoV);
    if (no_vstar_route) priorities.push_back(Card::RegidragoVstar);
    if (no_locked_payload_route) {
      priorities.push_back(Card::ProfessorBurnet);
      priorities.push_back(Card::Crispin);
      priorities.push_back(Card::MegaDragonite);
    }
    for (const Card target : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), target);
      if (it == state_.prizes.end()) continue;
      const Card chosen = *it;
      *it = Card::Gladion;  // Gladion goes into the prize pool, not the discard pile.
      state_.hand.push_back(chosen);
      shuffle(state_.prizes);
      trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER", "Looked at Prize cards and exchanged Gladion for " + std::string(name(chosen)) + ".");
      return true;
    }
    // Gladion must exchange itself with one Prize card even if the targeted collapse did not occur.
    const std::array<Card, 8> lowest_impact{Card::Dipplin, Card::Grass, Card::Fire, Card::Powerglass,
                                            Card::FieldBlower, Card::ChaoticSwell, Card::PathToPeak, Card::MawileGX};
    auto selected = state_.prizes.begin();
    for (const Card low : lowest_impact) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), low);
      if (it != state_.prizes.end()) { selected = it; break; }
    }
    const Card chosen = *selected;
    *selected = Card::Gladion;
    state_.hand.push_back(chosen);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER", "Target axis was not prized; completed the mandatory exchange with " + std::string(name(chosen)) + ".");
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
        // Only discard extra cards if they are in the explicit flexible DCI set or a dead Dipplin.
        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::MawileGX || *extra == Card::Guzma ||
                               *extra == Card::Channeler || *extra == Card::FieldBlower || *extra == Card::ChaoticSwell || *extra == Card::PathToPeak;
        if (!can_cycle) break;
        discard_from_hand(*extra, "Serena optional discard", "R-SERENA-01");
      }
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER", "Used Serena draw mode after discarding at least one card.");
    return true;
  }

  bool use_legacy_star() {
    if (state_.vstar_power_used || !has_vstar() || !ability_available_for_pokemon(Card::RegidragoVstar)) return false;
    // Use only when no direct payload outlet or direct connector remains and the discard-7 can create a JIT payload line.
    const bool only_random_payload_line = need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0 &&
                                          hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::QuickBall) == 0 && hand_count(Card::EarthenVessel) == 0 && hand_count(Card::UltraBall) == 0;
    if (!only_random_payload_line) return false;
    state_.vstar_power_used = true;
    outcome_.used_legacy_star = true;
    std::vector<Card> discarded;
    for (int card = 0; card < 7 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      state_.discard.push_back(top);
      state_.discarded_this_turn.push_back(top);
      discarded.push_back(top);
    }
    std::vector<Card> recovered;
    // Recover energy only if it enables a manual attachment this turn. Do not recover a strict-JIT payload.
    for (const Card desired : {Card::Grass, Card::Fire, Card::Crispin, Card::Arven}) {
      if (recovered.size() >= 2U) break;
      if (is_payload(desired) && strict_payload_timing()) continue;
      const auto it = std::find(state_.discard.begin(), state_.discard.end(), desired);
      if (it == state_.discard.end()) continue;
      if (desired == Card::Grass && grass_needed() == 0) continue;
      if (desired == Card::Fire && fire_needed() == 0) continue;
      state_.hand.push_back(*it);
      state_.discard.erase(it);
      recovered.push_back(desired);

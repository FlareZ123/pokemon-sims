  bool use_fss() {
    if (state_.vstar_power_used) return false;
    Pokemon* holder = nullptr;
    if (state_.active && state_.active->tool == Tool::ForestSealStone) holder = &*state_.active;
    if (holder == nullptr) {
      for (Pokemon& p : state_.bench) if (p.tool == Tool::ForestSealStone) { holder = &p; break; }
    }
    if (holder == nullptr || !is_pokemon_v(holder->card)) return false;
    const bool tate_solves_only_missing_axis = need_active_vstar() && !need_energy() && !need_payload() &&
                                               supporter_allowed() && hand_count(Card::TateLiza) > 0;
    const bool crispin_solves_only_missing_axis = need_energy() && !need_regi() && !need_vstar() &&
                                                  !need_active_vstar() && !need_payload() && supporter_allowed() &&
                                                  hand_count(Card::Crispin) > 0;
    const bool useful = !tate_solves_only_missing_axis && !crispin_solves_only_missing_axis &&
                        (should_play_steven() || need_regi() || need_vstar() || need_energy() || need_payload() || need_active_vstar());
    if (!useful) return false;
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Star Alchemy is a deck search; target policy runs after that legal inspection.
    record_deck_search_knowledge("Star Alchemy");
    Card target = fss_target_after_search_started();
    bool found = move_deck_to_hand(target);
    if (!found) {
      for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender,
                                  Card::ProfessorBurnet, Card::Arven, Card::MysteriousTreasure,
                                  Card::QuickBall, Card::EarthenVessel, Card::LatiasEx}) {
        if (move_deck_to_hand(fallback)) { target = fallback; found = true; break; }
      }
    }
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01", found ? "Searched any card: " + std::string(name(target)) + "." : "No card remained in deck.");
    return true;
  }

  bool play_crispin() {
    if (!supporter_allowed() || hand_count(Card::Crispin) == 0 || !need_energy() || target_regi() == nullptr) return false;

    // A one-type search still has value: Crispin puts that Energy into hand,
    // where the unused manual attachment can advance the missing GGF component.
    const bool deck_has_grass = deck_count_after_search_started(Card::Grass) > 0;
    const bool deck_has_fire = deck_count_after_search_started(Card::Fire) > 0;
    const bool known_relevant_energy = (grass_needed() > 0 && deck_has_grass) ||
                                       (fire_needed() > 0 && deck_has_fire);
    if (deck_seen_ && !known_relevant_energy) return false;
    if (!deck_seen_ && !might_be_unseen(Card::Grass) && !might_be_unseen(Card::Fire)) return false;

    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;

    // Crispin still inspects the whole deck if only one or no type exists.
    // Card text source: https://api.pokemontcg.io/v2/cards/sv7-133
    record_deck_search_knowledge("Crispin");
    if (!deck_has_grass || !deck_has_fire) {
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
    for (const Card target : wanted) {
      if (found.size() >= 3U) break;

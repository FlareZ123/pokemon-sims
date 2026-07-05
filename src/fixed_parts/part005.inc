    if (!second) return false;

    remove_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first);
    discard_from_hand(*second);
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx}) {
      const bool matches = (target == Card::RegidragoV && wants_regi) ||
                           (target == Card::RegidragoVstar && wants_vstar) ||
                           (target == Card::LatiasEx && wants_latias);
      if (matches && move_from_deck_to_hand(target)) break;
    }
    shuffle(state_.deck);
    return true;
  }

  bool play_evolution_incense() {
    if (item_locked() || hand_count(Card::EvolutionIncense) == 0 || !has_regi_v() || has_vstar() || hand_count(Card::RegidragoVstar) > 0) return false;
    remove_one(state_.hand, Card::EvolutionIncense);
    state_.discard.push_back(Card::EvolutionIncense);
    move_from_deck_to_hand(Card::RegidragoVstar);
    shuffle(state_.deck);
    return true;
  }

  bool play_earthen_vessel() {
    if (item_locked() || hand_count(Card::EarthenVessel) == 0) return false;
    const bool payload_discard = payload_window_open() && need_payload() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (!need_energy() && !payload_discard) return false;
    const auto discard = choose_discard(payload_discard, true);
    if (!discard) return false;
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(*discard);
    resolve_energy_search(grass_needed(), fire_needed(), 2);
    return true;
  }

  bool play_blender() {
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 || !payload_window_open() || !need_payload()) return false;
    remove_one(state_.hand, Card::BrilliantBlender);
    state_.discard.push_back(Card::BrilliantBlender);
    for (const Card target : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_from_deck_to_discard(target)) break;
    }
    shuffle(state_.deck);
    outcome_.used_blender = true;
    return true;
  }

  Card choose_fss_target_after_search() const {
    // Star Alchemy searches the deck, so card availability is examined only after the VSTAR Power is declared.
    if (!has_vstar()) return Card::RegidragoVstar;
    if (need_energy()) return Card::Crispin;
    if (need_payload() && !item_locked()) return Card::BrilliantBlender;
    if (need_payload()) return Card::ProfessorBurnet;
    if (needs_active_vstar() && !in_play(Card::LatiasEx)) return Card::LatiasEx;
    return Card::Arven;
  }

  bool use_fss() {
    if (item_locked() || state_.vstar_power_used || hand_count(Card::ForestSealStone) == 0 || !has_regi_v()) return false;
    Pokemon* holder = nullptr;
    if (state_.active && state_.active->card == Card::RegidragoV) holder = &*state_.active;
    if (holder == nullptr) {
      for (Pokemon& p : state_.bench) if (p.card == Card::RegidragoV) { holder = &p; break; }
    }
    if (holder == nullptr || holder->forest_seal || holder->powerglass) return false;

    const bool steven_bridge = state_.turn == 1 && !scenario_.going_first && has_regi_v() &&
                               (hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Crispin) == 0 || hand_count(Card::BrilliantBlender) == 0) &&
                               possibly_unseen(Card::StevensResolve);
    const bool immediate_axis = !has_vstar() || need_energy() || need_payload() || needs_active_vstar();
    if (!steven_bridge && !immediate_axis) return false;

    remove_one(state_.hand, Card::ForestSealStone);
    holder->forest_seal = true;
    state_.vstar_power_used = true;
    outcome_.used_fss = true;

    const Card preferred = steven_bridge ? Card::StevensResolve : choose_fss_target_after_search();
    if (!move_from_deck_to_hand(preferred)) {
      for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender, Card::ProfessorBurnet, Card::Arven, Card::LatiasEx}) {
        if (move_from_deck_to_hand(fallback)) break;
      }
    }
    shuffle(state_.deck);
    return true;
  }

  bool crispin_can_plausibly_attach() const { return possibly_unseen(Card::Grass) && possibly_unseen(Card::Fire); }

  bool play_crispin() {
    Pokemon* regi = primary_regi();
    if (!supporter_allowed() || hand_count(Card::Crispin) == 0 || regi == nullptr || !need_energy() || !crispin_can_plausibly_attach()) return false;
    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;

    const bool have_grass = deck_count(Card::Grass) > 0;
    const bool have_fire = deck_count(Card::Fire) > 0;
    if (!have_grass || !have_fire) {
      // Legal degraded resolution: at most one different-type card can be found, so it goes to hand.
      if (have_grass) move_from_deck_to_hand(Card::Grass);
      else if (have_fire) move_from_deck_to_hand(Card::Fire);
      shuffle(state_.deck);
      return true;
    }

    const Card attach = grass_needed() > 0 ? Card::Grass : Card::Fire;
    const Card to_hand = attach == Card::Grass ? Card::Fire : Card::Grass;
    remove_one(state_.deck, attach);
    remove_one(state_.deck, to_hand);
    if (attach == Card::Grass) ++regi->grass;
    else ++regi->fire;
    state_.hand.push_back(to_hand);
    shuffle(state_.deck);
    return true;
  }

  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !has_regi_v()) return false;
    const bool needs_vstar = !has_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool needs_crispin = need_energy() && hand_count(Card::Crispin) == 0;
    const bool needs_outlet = need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0;
    if (static_cast<int>(needs_vstar) + static_cast<int>(needs_crispin) + static_cast<int>(needs_outlet) < 2) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;

    const Card outlet = item_locked() || (scenario_.locks == LockMode::TurnTwoItem && state_.turn == 1) ? Card::ProfessorBurnet : Card::BrilliantBlender;
    std::vector<Card> package;
    if (!has_vstar() && hand_count(Card::RegidragoVstar) == 0) package.push_back(Card::RegidragoVstar);
    if (need_energy() && hand_count(Card::Crispin) == 0) package.push_back(Card::Crispin);
    if (need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0) package.push_back(outlet);
    for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, outlet, Card::MysteriousTreasure, Card::EarthenVessel}) {
      if (package.size() >= 3U) break;
      if (std::find(package.begin(), package.end(), fallback) == package.end()) package.push_back(fallback);
    }
    for (const Card target : package) move_from_deck_to_hand(target);
    shuffle(state_.deck);
    state_.turn_ended = true;
    return true;
  }

  bool play_arven() {
    if (!supporter_allowed() || item_locked() || hand_count(Card::Arven) == 0) return false;
    const bool wants_vstar = has_regi_v() && !has_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool wants_energy = need_energy();
    const bool wants_payload = need_payload();
    if (!wants_vstar && !wants_energy && !wants_payload) return false;

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;

    Card item = wants_vstar ? Card::MysteriousTreasure : (wants_energy ? Card::EarthenVessel : Card::BrilliantBlender);
    if (item_locked()) item = Card::BrilliantBlender;  // It may be held, though it cannot be played under the current lock.
    move_from_deck_to_hand(item);
    if (has_regi_v() && !state_.vstar_power_used) move_from_deck_to_hand(Card::ForestSealStone);
    shuffle(state_.deck);
    return true;
  }

  bool play_burnet() {
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0 || !payload_window_open() || !need_payload()) return false;
    remove_one(state_.hand, Card::ProfessorBurnet);
    state_.discard.push_back(Card::ProfessorBurnet);
    state_.supporter_used = true;
    for (const Card target : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_from_deck_to_discard(target)) break;
    }
    shuffle(state_.deck);
    return true;
  }

  bool should_play_gladion() const {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0) return false;
    const bool missing_regi = !has_regi() && hand_count(Card::RegidragoV) == 0 && hand_count(Card::QuickBall) == 0 && hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::UltraBall) == 0 && hand_count(Card::Arven) == 0;
    const bool missing_vstar = has_regi_v() && !has_vstar() && hand_count(Card::RegidragoVstar) == 0 && hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::EvolutionIncense) == 0 && hand_count(Card::UltraBall) == 0 && hand_count(Card::Arven) == 0 && hand_count(Card::ForestSealStone) == 0;
    const bool missing_outlet = need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0 && item_locked();
    return missing_regi || missing_vstar || missing_outlet;
  }

  bool play_gladion() {
    if (!should_play_gladion()) return false;
    remove_one(state_.hand, Card::Gladion);
    state_.discard.push_back(Card::Gladion);
    state_.supporter_used = true;

    const std::array<Card, 6> priorities{Card::RegidragoVstar, Card::RegidragoV, Card::ProfessorBurnet, Card::BrilliantBlender, Card::Crispin, Card::MegaDragonite};
    for (const Card target : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), target);
      if (it == state_.prizes.end()) continue;
      state_.hand.push_back(*it);
      *it = Card::Gladion;
      shuffle(state_.prizes);
      return true;
    }
    // No critical target is prized. Gladion remains in the discard after the legal resolution.
    return true;
  }

  bool play_serena() {
    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;
    const bool payload_discard = payload_window_open() && need_payload() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (!payload_discard && state_.hand.size() >= 5U) return false;

    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    const auto first = choose_discard(payload_discard, true);
    if (!first) {
      state_.hand.push_back(Card::Serena);
      remove_one(state_.discard, Card::Serena);
      return false;
    }

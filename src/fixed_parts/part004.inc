      if (!remove_one(state_.deck, choice)) {
        // A search effect may select a different needed Basic Energy after revealing the deck.
        const Card alternative = choice == Card::Grass ? Card::Fire : Card::Grass;
        if (!remove_one(state_.deck, alternative)) break;
        choice = alternative;
      }
      picked.push_back(choice);
      if (choice == Card::Grass && remaining_grass > 0) --remaining_grass;
      if (choice == Card::Fire && remaining_fire > 0) --remaining_fire;
    }
    for (const Card target : picked) state_.hand.push_back(target);
    shuffle(state_.deck);
  }

  Card supporter_target_after_search() const {
    // This function is called while resolving Wonder Tag, so examining the deck is legal.
    if (state_.turn == 1 && !scenario_.going_first && has_regi_v() && hand_count(Card::StevensResolve) == 0 && deck_count(Card::StevensResolve) > 0) {
      return Card::StevensResolve;
    }
    if (need_energy() && deck_count(Card::Crispin) > 0) return Card::Crispin;
    if (need_payload() && (item_locked() || hand_count(Card::BrilliantBlender) == 0) && deck_count(Card::ProfessorBurnet) > 0) return Card::ProfessorBurnet;
    if (!has_vstar() && deck_count(Card::Arven) > 0) return Card::Arven;
    if (deck_count(Card::Arven) > 0) return Card::Arven;
    if (deck_count(Card::TateLiza) > 0) return Card::TateLiza;
    return Card::Crispin;
  }

  void resolve_entry_ability(const Card card) {
    if (card == Card::TapuLeleGX) {
      if (!ability_active(card)) return;
      const Card target = supporter_target_after_search();
      if (move_from_deck_to_hand(target)) shuffle(state_.deck);
      return;
    }
    if (card == Card::Oricorio) {
      // Vital Dance is an entry Ability. Oricorio has no Rule Box, so Path-style locks do not stop it.
      resolve_energy_search(grass_needed(), fire_needed(), 2);
    }
  }

  std::optional<Card> choose_discard(const bool allow_jit_payload, const bool include_matchup_flex) const {
    if (allow_jit_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(payload) > 0) return payload;
      }
    }

    if (include_matchup_flex && scenario_.dci != DciProfile::StrictJit) {
      for (const Card card : {Card::MawileGX, Card::Dipplin, Card::Guzma, Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer, Card::RoseannesBackup,
                              Card::Lusamine, Card::ProfessorTuro, Card::Powerglass}) {
        if (hand_count(card) > 0) return card;
      }
    }

    if (hand_count(Card::RegidragoV) > 0 && (has_regi() || hand_count(Card::RegidragoV) >= 2)) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && (has_vstar() || hand_count(Card::RegidragoVstar) >= 2)) return Card::RegidragoVstar;

    // Never use hidden deck/prize contents to label an Energy disposable. Only a visible hand surplus is safe.
    if (hand_count(Card::Grass) > grass_needed() + 1) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed() + 1) return Card::Fire;

    if (scenario_.dci == DciProfile::NoDiscardControl) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(payload) > 0) return payload;
      }
    }
    return std::nullopt;
  }

  bool discard_from_hand(const Card card) {
    if (!remove_one(state_.hand, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    return true;
  }

  bool attach_manual() {
    if (state_.manual_attachment_used) return false;
    Pokemon* regi = primary_regi();
    if (regi == nullptr) return false;
    Card desired = regi->grass < 2 ? Card::Grass : (regi->fire < 1 ? Card::Fire : Card::Grass);
    if (hand_count(desired) == 0) return false;
    remove_one(state_.hand, desired);
    if (desired == Card::Grass) ++regi->grass;
    else ++regi->fire;
    state_.manual_attachment_used = true;
    return true;
  }

  bool evolve() {
    if (state_.turn == 1 || hand_count(Card::RegidragoVstar) == 0) return false;
    auto eligible = [this](const Pokemon& p) { return p.card == Card::RegidragoV && p.entered_turn < state_.turn; };
    if (state_.active && eligible(*state_.active)) {
      remove_one(state_.hand, Card::RegidragoVstar);
      state_.active->card = Card::RegidragoVstar;
      return true;
    }
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), eligible);
    if (it == state_.bench.end()) return false;
    remove_one(state_.hand, Card::RegidragoVstar);
    it->card = Card::RegidragoVstar;
    return true;
  }

  bool promote_vstar_with_latias() {
    if (active_is_vstar() || !in_play(Card::LatiasEx) || !ability_active(Card::LatiasEx) || !state_.active) return false;
    if (!is_basic(state_.active->card)) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    std::swap(*state_.active, *it);
    return true;
  }

  bool play_tate_switch() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || !needs_active_vstar() || !state_.active) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    std::swap(*state_.active, *it);
    return true;
  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0 || has_regi()) return false;
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::UltraBall) > 0 || hand_count(Card::Arven) > 0) return false;
    // The decision to play does not inspect prizes. The target is selected after the effect reveals them.
    remove_one(state_.hand, Card::HisuianHeavyBall);
    state_.discard.push_back(Card::HisuianHeavyBall);
    const std::array<Card, 5> target_order{Card::RegidragoV, Card::TapuLeleGX, Card::Oricorio, Card::LatiasEx, Card::MawileGX};
    for (const Card target : target_order) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), target);
      if (it == state_.prizes.end()) continue;
      const Card found = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(found);
      shuffle(state_.prizes);
      return true;
    }
    // Legal resolution: no Basic Pokémon is chosen. Heavy Ball stays in discard.
    return true;
  }

  bool play_mysterious_treasure() {
    if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;
    const bool payload_discard = payload_window_open() && need_payload() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool wants_regi = !has_regi();
    const bool wants_vstar = has_regi_v() && !has_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool wants_latias = needs_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool wants_tapu = !state_.supporter_used && hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX);
    if (!payload_discard && !wants_regi && !wants_vstar && !wants_latias && !wants_tapu) return false;
    const auto discard = choose_discard(payload_discard, true);
    if (!discard) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(*discard);

    const std::array<Card, 8> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::TapuLeleGX,
                                      Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX};
    for (const Card target : targets) {
      const bool matches_need = (target == Card::RegidragoV && wants_regi) ||
                                (target == Card::RegidragoVstar && wants_vstar) ||
                                (target == Card::LatiasEx && wants_latias) ||
                                (target == Card::TapuLeleGX && wants_tapu) ||
                                (payload_discard && is_payload(target));
      if (matches_need && move_from_deck_to_hand(target)) {
        shuffle(state_.deck);
        return true;
      }
    }
    shuffle(state_.deck);
    return true;
  }

  bool play_quick_ball() {
    if (item_locked() || hand_count(Card::QuickBall) == 0) return false;
    const bool payload_discard = payload_window_open() && need_payload() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool wants_regi = !has_regi();
    const bool wants_latias = needs_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool wants_tapu = !state_.supporter_used && hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX);
    if (!payload_discard && !wants_regi && !wants_latias && !wants_tapu) return false;
    const auto discard = choose_discard(payload_discard, true);
    if (!discard) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(*discard);

    const std::array<Card, 5> targets{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::DialgaGX};
    for (const Card target : targets) {
      const bool matches_need = (target == Card::RegidragoV && wants_regi) ||
                                (target == Card::LatiasEx && wants_latias) ||
                                (target == Card::TapuLeleGX && wants_tapu) ||
                                (payload_discard && target == Card::DialgaGX);
      if (matches_need && move_from_deck_to_hand(target)) {
        shuffle(state_.deck);
        return true;
      }
    }
    shuffle(state_.deck);
    return true;
  }

  bool play_ultra_ball() {
    if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
    const bool wants_regi = !has_regi();
    const bool wants_vstar = has_regi_v() && !has_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool wants_latias = needs_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    if (!wants_regi && !wants_vstar && !wants_latias) return false;

    const auto first = choose_discard(payload_window_open() && need_payload(), true);
    if (!first) return false;
    remove_one(state_.hand, *first);
    const auto second = choose_discard(payload_window_open() && need_payload() && !is_payload(*first), true);
    state_.hand.push_back(*first);


  bool discard_from_hand(const Card card, std::string_view why, std::string_view rules) {
    if (!remove_one(state_.hand, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    trace("DISCARD", rules, std::string(name(card)) + " (" + std::string(why) + ")");
    return true;
  }

  // Tracks a card leaving the discard pile in the same turn, so strict-JIT state
  // cannot count a payload that was discarded and then recovered before the ready check.
  bool recover_discard_to_hand(const Card card) {
    const auto discard_it = std::find(state_.discard.begin(), state_.discard.end(), card);
    if (discard_it == state_.discard.end()) return false;
    state_.hand.push_back(*discard_it);
    state_.discard.erase(discard_it);
    const auto turn_it = std::find(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), card);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    return true;
  }

  bool bench_from_hand(const Card card, const bool resolve_entry) {
    if (bench_space() <= 0 || !is_basic(card) || !remove_one(state_.hand, card)) return false;
    state_.bench.push_back(Pokemon{card, state_.turn});
    trace("BENCH", "R-GAME-BENCH", std::string(name(card)) + " from hand.");
    if (resolve_entry) resolve_entry_ability(card);
    return true;
  }

  bool can_play_payload_this_turn() const { return payload_window_open() && need_payload(); }

  std::optional<Card> choose_discard(const bool permit_payload, const bool flex_fodder) const {
    // P-DCI-STRICT and P-DCI-FLEX, documented in RULES_TRACEABILITY.md.
    if (permit_payload) {
      for (const Card card : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }

    // Dipplin cannot be legally put into play without Applin, which is absent from this deck.
    if (hand_count(Card::Dipplin) > 0) return Card::Dipplin;

    if (hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) >= 2) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && has_vstar()) return Card::RegidragoVstar;

    // Extra in-hand Energy is observable; no deck / Prize identities are used to call Energy expendable.
    if (hand_count(Card::Grass) > grass_needed() + 1) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed() + 1) return Card::Fire;

    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
      for (const Card card : {Card::MawileGX, Card::Guzma, Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                              Card::RoseannesBackup, Card::Lusamine, Card::ProfessorTuro,
                              Card::Powerglass}) {
        if (hand_count(card) > 0) return card;
      }
    }

    if (scenario_.dci == DciProfile::NoDiscardControl) {
      for (const Card card : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }
    return std::nullopt;
  }

  bool is_legal_cost_available(const bool permit_payload, const bool flex_fodder) const {
    return choose_discard(permit_payload, flex_fodder).has_value();
  }

  // P-CONNECTOR-01: do not spend a search connector for Oricorio while a present Supporter or Item
  // already supplies the needed Energy axis this turn.
  bool needs_oricorio_connector() const {
    if (!need_energy()) return false;
    if (supporter_allowed() && hand_count(Card::Crispin) > 0) return false;
    if (!item_locked() && hand_count(Card::EarthenVessel) > 0 && is_legal_cost_available(can_play_payload_this_turn(), true)) return false;
    const int accessible_now = hand_count(Card::Grass) + hand_count(Card::Fire);
    return accessible_now < grass_needed() + fire_needed();
  }

  bool needs_tapu_connector() const {
    if (!supporter_allowed()) return false;
    const bool needs_t1_steven = state_.turn == 1 && !scenario_.going_first && has_regi_v() && should_play_steven() && hand_count(Card::StevensResolve) == 0;
    const bool needs_crispin = need_energy() && hand_count(Card::Crispin) == 0;
    const bool needs_burnet = need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) == 0;
    const bool needs_arven = has_regi_v() && !has_vstar() && hand_count(Card::Arven) == 0 && hand_count(Card::EvolutionIncense) == 0 && hand_count(Card::MysteriousTreasure) == 0;
    return needs_t1_steven || needs_crispin || needs_burnet || needs_arven;
  }

  bool bench_oricorio_if_useful() {
    if (hand_count(Card::Oricorio) == 0 || in_play(Card::Oricorio) || bench_space() <= 0 || !needs_oricorio_connector()) return false;
    return bench_from_hand(Card::Oricorio, true);
  }

  bool bench_tapu_if_useful() {
    if (hand_count(Card::TapuLeleGX) == 0 || in_play(Card::TapuLeleGX) || bench_space() <= 0 || !needs_tapu_connector()) return false;
    return bench_from_hand(Card::TapuLeleGX, true);
  }

  void resolve_entry_ability(const Card card) {
    if (card == Card::TapuLeleGX) {
      if (!ability_available_for_pokemon(Card::TapuLeleGX)) {
        trace("ABILITY BLOCKED", "R-PATH-01", "Tapu Lele-GX Wonder Tag is suppressed by the Rule Box Ability lock.");
        return;
      }
      // The search permits inspecting the deck to select a Supporter.
      const Card target = choose_supporter_after_search_started();
      if (move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("WONDER TAG", "R-TAPU-01", "Searched and revealed " + std::string(name(target)) + ".");
      } else {
        shuffle(state_.deck);
        trace("WONDER TAG", "R-TAPU-01", "No selected Supporter was in deck after legal search.");
      }
      return;
    }
    if (card == Card::Oricorio) {
      // Oricorio has no Rule Box; Path-style rule-box suppression does not stop Vital Dance.
      search_energy_to_hand(2, "R-ORICORIO-01", "Vital Dance");
    }
  }

  Card choose_supporter_after_search_started() const {
    // This is called inside Wonder Tag after the deck is being legally searched.
    if (state_.turn == 1 && !scenario_.going_first && has_regi_v() && should_play_steven() &&
        deck_count_after_search_started(Card::StevensResolve) > 0) return Card::StevensResolve;
    if (need_energy() && deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;
    if (need_payload() && item_locked() && deck_count_after_search_started(Card::ProfessorBurnet) > 0) return Card::ProfessorBurnet;
    if (has_regi_v() && !has_vstar() && deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    if (deck_count_after_search_started(Card::Serena) > 0) return Card::Serena;
    if (deck_count_after_search_started(Card::TateLiza) > 0) return Card::TateLiza;
    return Card::Crispin;
  }

  std::vector<Card> choose_energy_targets_after_search_started(const int max_cards) const {
    std::vector<Card> targets;
    int g_need = grass_needed();
    int f_need = fire_needed();
    int g_available = deck_count_after_search_started(Card::Grass);
    int f_available = deck_count_after_search_started(Card::Fire);
    while (static_cast<int>(targets.size()) < max_cards) {
      Card wanted = Card::Grass;
      if (g_need > 0 && f_need > 0) {
        wanted = std::find(targets.begin(), targets.end(), Card::Grass) == targets.end() ? Card::Grass : Card::Fire;
      } else if (g_need > 0) {
        wanted = Card::Grass;
      } else if (f_need > 0) {
        wanted = Card::Fire;
      } else {
        break;
      }
      if ((wanted == Card::Grass && g_available == 0) || (wanted == Card::Fire && f_available == 0)) {
        const Card other = wanted == Card::Grass ? Card::Fire : Card::Grass;
        if ((other == Card::Grass && g_available == 0) || (other == Card::Fire && f_available == 0)) break;
        wanted = other;
      }
      targets.push_back(wanted);
      if (wanted == Card::Grass) {
        --g_available;
        if (g_need > 0) --g_need;
      } else {
        --f_available;
        if (f_need > 0) --f_need;
      }
    }
    return targets;
  }

  void search_energy_to_hand(const int max_cards, std::string_view rules, std::string_view source) {
    const std::vector<Card> targets = choose_energy_targets_after_search_started(max_cards);
    for (const Card target : targets) {
      if (!move_deck_to_hand(target)) throw std::logic_error("energy search target disappeared");
    }
    shuffle(state_.deck);
    trace(std::string(source), rules, "Searched up to " + std::to_string(max_cards) + " Basic Energy: " + join_cards(targets) + ".");
  }

  bool attach_manual() {
    if (state_.manual_energy_used) return false;
    Pokemon* target = target_regi();
    if (target == nullptr || !need_energy()) return false;
    Card wanted = Card::Grass;
    if (grass_needed() > 0 && hand_count(Card::Grass) > 0) {
      wanted = Card::Grass;
    } else if (fire_needed() > 0 && hand_count(Card::Fire) > 0) {
      wanted = Card::Fire;
    } else {
      // Do not use the once-per-turn attachment on an Energy type that does not advance GGF.
      return false;
    }
    remove_one(state_.hand, wanted);
    if (wanted == Card::Grass) ++target->grass; else ++target->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY", std::string(name(wanted)) + " manually to " + std::string(name(target->card)) + ".");
    return true;
  }

  bool evolve_best_regi() {
    if (state_.turn == 1 || hand_count(Card::RegidragoVstar) == 0) return false;
    Pokemon* best = nullptr;
    int best_score = -100000;
    if (state_.active && state_.active->card == Card::RegidragoV && state_.active->entered_turn < state_.turn) {
      best = &*state_.active;
      best_score = score_regi(*best, true);
    }
    for (Pokemon& p : state_.bench) {
      if (p.card != Card::RegidragoV || p.entered_turn >= state_.turn) continue;
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    if (best == nullptr) return false;
    remove_one(state_.hand, Card::RegidragoVstar);
    best->card = Card::RegidragoVstar;
    trace("EVOLVE", "R-GAME-EVOLVE", "Evolved " + std::string(name(Card::RegidragoV)) + " into Regidrago VSTAR.");
    return true;
  }

  bool retreat_to_benched_vstar_with_latias() {
    if (!need_active_vstar() || !can_free_retreat_with_latias()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    std::swap(*state_.active, *it);
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT", "Latias ex gives the Basic Active no Retreat Cost; promoted Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_switch() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || !need_active_vstar()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    std::swap(*state_.active, *it);
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza switch mode: promoted Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_draw() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || state_.hand.size() >= 4U) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int draw = 0; draw < 5; ++draw) draw_card("Tate & Liza");
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza draw mode: shuffled remaining hand and drew 5.");
    return true;
  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0 || has_any_regi()) return false;
    // Let live normal Basic-search connectors act first. This preserves Heavy Ball for a collapsed Basic prize state.
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::UltraBall) > 0 || hand_count(Card::Arven) > 0) return false;
    remove_one(state_.hand, Card::HisuianHeavyBall);
    const std::array<Card, 6> priorities{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::MawileGX, Card::DialgaGX};
    for (const Card desired : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), desired);
      if (it == state_.prizes.end()) continue;
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM", "Looked at Prizes and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM", "Looked at Prizes; did not reveal a useful Basic, so Heavy Ball was discarded.");
    return true;
  }

  bool need_regi() const { return !has_any_regi(); }
  bool need_vstar() const { return has_regi_v() && !has_vstar(); }

  bool has_other_payload_outlet(const Card excluded) const {
    // Only visible cards are used here. The current search card cannot be counted as its own later discard outlet.
    const auto present = [this, excluded](const Card card) { return card != excluded && hand_count(card) > 0; };
    return present(Card::MysteriousTreasure) || present(Card::QuickBall) || present(Card::UltraBall) ||
           present(Card::EarthenVessel) || present(Card::Serena) || present(Card::BrilliantBlender) ||
           (supporter_allowed() && present(Card::ProfessorBurnet));
  }


  Card fallback_mysterious_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  Card fallback_quick_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX,
                              Card::MawileGX, Card::DialgaGX}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }

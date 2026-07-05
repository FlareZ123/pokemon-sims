      state_.prizes.push_back(state_.deck.back());
      state_.deck.pop_back();
    }

    if (trace_ != nullptr && trace_->enabled) {
      trace_->note("===== SIMULATE THIS =====");
      trace_->note("Scenario: " + scenario_.label + " | DCI: " + dci_name() + " | Locks: " + lock_name());
      trace_->note("Mulligans: " + std::to_string(outcome_.mulligans));
      trace_->note("Opening player-known state: " + state_line());
      trace_->note("DEBUG ONLY, never used by policy before a legal reveal/search: prizes=" + join_cards(state_.prizes));
      trace_->note("Policy rule: strict and matchup-flex JIT require a payload to enter discard in the same player turn as the ready-state check.");
    }
  }

  std::string dci_name() const {
    switch (scenario_.dci) {
      case DciProfile::StrictJit: return "strict-JIT";
      case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
      case DciProfile::NoDiscardControl: return "no-discard-control";
    }
    return "unknown";
  }

  std::string lock_name() const {
    switch (scenario_.locks) {
      case LockMode::None: return "none";
      case LockMode::TurnTwoItem: return "turn-two Item lock";
      case LockMode::FullItem: return "full Item lock";
      case LockMode::FullRuleBoxAbility: return "full Rule Box Ability lock";
      case LockMode::FullCombined: return "full Item + Rule Box Ability lock";
    }
    return "unknown";
  }

  void choose_opening_active() {
    // R-GAME-SETUP: only Basics can be chosen; this policy preserves entry-Abilities where possible.
    for (const Card candidate : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::MawileGX, Card::DialgaGX, Card::TapuLeleGX}) {
      if (remove_one(state_.hand, candidate)) {
        state_.active = Pokemon{candidate, 0};
        outcome_.started_regi = candidate == Card::RegidragoV;
        return;
      }
    }
    throw std::runtime_error("opening hand has no Basic after mulligan validation");
  }

  void choose_opening_bench() {
    // R-TAPU-01/R-ORICORIO-01: their on-Bench effects require a play from hand during a turn.
    if (bench_space() > 0 && hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) < 2) {
      remove_one(state_.hand, Card::RegidragoV);
      state_.bench.push_back(Pokemon{Card::RegidragoV, 0});
    }
    // R-LATIAS-01 is continuous. Setup Bench placement is useful if the Active is not Regidrago V.
    if (bench_space() > 0 && hand_count(Card::LatiasEx) > 0 && !in_play(Card::LatiasEx) && !active_is_regi_v()) {
      remove_one(state_.hand, Card::LatiasEx);
      state_.bench.push_back(Pokemon{Card::LatiasEx, 0});
    }
  }

  bool active_is_regi_v() const { return state_.active && state_.active->card == Card::RegidragoV; }

  bool move_deck_to_hand(const Card card) {
    if (!remove_one(state_.deck, card)) return false;
    state_.hand.push_back(card);
    return true;
  }

  bool move_deck_to_discard(const Card card) {
    if (!remove_one(state_.deck, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    return true;
  }

  bool discard_from_hand(const Card card, std::string_view why, std::string_view rules) {
    if (!remove_one(state_.hand, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    trace("DISCARD", rules, std::string(name(card)) + " (" + std::string(why) + ")");
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

  std::vector<Card> lusamine_recovery_targets() const {
    std::vector<Card> targets;
    for (const Card preferred : {Card::Crispin, Card::ProfessorBurnet, Card::Arven,
                                 Card::StevensResolve, Card::Gladion, Card::Serena,
                                 Card::TateLiza}) {
      if (targets.size() >= 2U) break;
      if (count_of(state_.discard, preferred) > 0 &&
          std::find(targets.begin(), targets.end(), preferred) == targets.end()) {
        targets.push_back(preferred);
      }
    }
    return targets;
  }

  bool play_lusamine() {
    if (!supporter_allowed() || hand_count(Card::Lusamine) == 0) return false;
    const std::vector<Card> targets = lusamine_recovery_targets();
    if (targets.empty()) return false;
    remove_one(state_.hand, Card::Lusamine);
    state_.discard.push_back(Card::Lusamine);
    state_.supporter_used = true;
    for (const Card target : targets) {
      if (!remove_one(state_.discard, target)) throw std::logic_error("Lusamine recovery target disappeared");
      state_.hand.push_back(target);
    }
    trace("PLAY SUPPORTER", "R-LUSAMINE-01; R-GAME-SUPPORTER; P-FUTURE-01",
          "Recovered from discard: " + join_cards(targets) + ".");
    return true;
  }

  bool play_serena_zero_draw_if_useful() {
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-164
    // The printed draw mode permits zero to three discards. Use zero only when
    // it increases hand size and no policy-safe discard route is available.
    if (!supporter_allowed() || hand_count(Card::Serena) == 0 || state_.hand.size() > 4U) return false;
    const bool permit_payload = can_play_payload_this_turn() &&
                                std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (choose_discard(permit_payload, true).has_value()) return false;
    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER",
          "Used Serena draw mode with zero discards because it increased a dead hand.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (need_active_vstar() && !need_energy() && !need_payload() && play_tate_switch()) return;

    if (prizes_known() && play_gladion()) return;
    if (play_steven()) return;
    if (need_energy() && hand_count(Card::Crispin) > 0 && play_crispin()) return;
    if (need_payload() && item_locked() && hand_count(Card::ProfessorBurnet) > 0 && play_professor_burnet()) return;
    if (hand_count(Card::Arven) > 0 && play_arven()) return;
    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::BrilliantBlender) == 0 && play_professor_burnet()) return;
    if (hand_count(Card::Serena) > 0 && play_serena()) return;
    if (hand_count(Card::Serena) > 0 && play_serena_zero_draw_if_useful()) return;
    if (hand_count(Card::TateLiza) > 0 && play_tate_draw()) return;
    if (need_active_vstar() && play_tate_switch()) return;

    // Lusamine is a delayed recovery line and must not preempt a live direct connector.
    if (play_lusamine()) return;
    play_gladion();
  }

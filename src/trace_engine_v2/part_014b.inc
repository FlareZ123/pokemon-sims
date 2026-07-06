  std::vector<Card> lusamine_recovery_targets() const {
    // Lusamine may recover up to two Supporter and/or Stadium cards: https://api.pokemontcg.io/v2/cards/sm4-96
    // Recover only cards that advance an unresolved setup axis on the following
    // Supporter turn. This avoids a static card order that can retrieve dead
    // Energy or payload connectors while VSTAR or Active position is still missing.
    std::vector<Card> targets;
    const auto recover_once = [this, &targets](const Card card) {
      if (targets.size() < 2U && count_of(state_.discard, card) > 0 &&
          std::find(targets.begin(), targets.end(), card) == targets.end()) {
        targets.push_back(card);
      }
    };

    const bool known_gladion_axis = prizes_known() &&
        ((need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         (need_regi() && bench_space() > 0 && prize_count_after_reveal(Card::RegidragoV) > 0) ||
         final_prized_energy_for_gladion().has_value());
    const bool arven_vstar_axis = need_vstar() && !item_locked() &&
                                  might_be_unseen(Card::EvolutionIncense);
    const bool arven_regi_axis = need_regi() && bench_space() > 0 && !item_locked() &&
                                 is_legal_cost_available(can_play_payload_this_turn(), true) &&
                                 (might_be_unseen(Card::QuickBall) || might_be_unseen(Card::MysteriousTreasure));
    const bool arven_energy_axis = need_energy() && !item_locked() &&
                                   is_legal_cost_available(can_play_payload_this_turn(), true) &&
                                   might_be_unseen(Card::EarthenVessel);
    const bool arven_payload_axis = need_payload() && !item_locked() &&
                                    might_be_unseen(Card::BrilliantBlender);

    if (known_gladion_axis) recover_once(Card::Gladion);
    if (arven_vstar_axis || arven_regi_axis) recover_once(Card::Arven);
    if (need_active_vstar()) recover_once(Card::TateLiza);
    if (need_energy() && crispin_can_advance_energy_axis()) recover_once(Card::Crispin);
    if (need_payload()) recover_once(Card::ProfessorBurnet);
    if (arven_energy_axis || arven_payload_axis) recover_once(Card::Arven);
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
    if (hand_count(Card::TateLiza) > 0 && play_tate_draw()) return;
    if (need_active_vstar() && play_tate_switch()) return;

    // Lusamine is a delayed recovery line and must not preempt a live direct connector.
    if (play_lusamine()) return;
    play_gladion();
  }
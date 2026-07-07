  std::vector<Card> lusamine_recovery_targets() const {
    // Lusamine says to put 2 Supporter and/or Stadium cards from discard into hand:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // A policy-first recovery requires one card that advances an unresolved setup
    // axis. Once that card makes Lusamine worth playing, the second selection is
    // mandatory and uses a deterministic low-impact eligible card.
    std::vector<Card> targets;
    const auto recover_once = [this, &targets](const Card card) {
      if (targets.size() < 2U && count_of(state_.discard, card) > count_of(targets, card)) {
        targets.push_back(card);
      }
    };

    const bool legal_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);
    const bool known_gladion_axis = prizes_known() &&
        ((need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         (need_regi() && bench_space() > 0 && prize_count_after_reveal(Card::RegidragoV) > 0) ||
         final_prized_energy_for_gladion().has_value());
    const bool arven_vstar_axis = need_vstar() && !item_locked() &&
                                  (might_be_unseen(Card::EvolutionIncense) ||
                                   (legal_search_cost && might_be_unseen(Card::MysteriousTreasure)));
    const bool arven_regi_axis = need_regi() && bench_space() > 0 && !item_locked() &&
                                 legal_search_cost &&
                                 (might_be_unseen(Card::QuickBall) || might_be_unseen(Card::MysteriousTreasure));
    const bool arven_energy_axis = need_energy() && !item_locked() &&
                                   legal_search_cost && might_be_unseen(Card::EarthenVessel);
    const bool arven_payload_axis = need_payload() && !item_locked() &&
                                    might_be_unseen(Card::BrilliantBlender);

    if (known_gladion_axis) recover_once(Card::Gladion);
    if (arven_vstar_axis || arven_regi_axis) recover_once(Card::Arven);
    if (need_active_vstar()) recover_once(Card::TateLiza);
    if (need_energy() && crispin_can_advance_energy_axis()) recover_once(Card::Crispin);
    if (need_payload()) recover_once(Card::ProfessorBurnet);
    if (arven_energy_axis || arven_payload_axis) recover_once(Card::Arven);

    if (targets.empty()) return targets;

    // Lusamine requires exactly 2 eligible cards. Its printed effect has no "up to"
    // permission, so one-target recovery is not a legal resolution: https://api.pokemontcg.io/v2/cards/sm4-96
    // A failed exact-count check makes play_lusamine retain the Supporter in hand.
    for (const Card fallback : {Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                                Card::RoseannesBackup, Card::ProfessorTuro, Card::Channeler,
                                Card::Guzma, Card::Serena, Card::TateLiza, Card::StevensResolve,
                                Card::Arven, Card::Crispin, Card::ProfessorBurnet, Card::Gladion}) {
      recover_once(fallback);
    }
    if (targets.size() != 2U) return {};
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

    // Exact Serena text: discard up to 3 cards, but "You must discard at least 1 card."
    // A zero-discard draw is illegal: https://api.pokemontcg.io/v2/cards/swsh12-164
    // This route requires an in-hand payload as that mandatory discard, then resolves
    // Serena before Prize recovery spends the only Supporter play.
    const bool serena_completes_only_missing_axis = need_payload() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_active_vstar() && can_play_payload_this_turn() &&
        hand_count(Card::Serena) > 0 && std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (serena_completes_only_missing_axis && play_serena()) return;

    if (prizes_known() && play_gladion()) return;
    if (play_steven()) return;
    if (need_energy() && hand_count(Card::Crispin) > 0 && play_crispin()) return;
    // Brilliant Blender is an Item that searches the deck and discards the current-turn payload: https://api.pokemontcg.io/v2/cards/sv8-164
    // Preserve Burnet's one Supporter play while that live Item line supplies the same payload axis: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://www.pokemon.com/us/pokemon-tcg/rules
    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && !has_live_blender_payload_line() && play_professor_burnet()) return;
    if (hand_count(Card::Arven) > 0 && play_arven()) return;
    if (hand_count(Card::Serena) > 0 && play_serena()) return;
    if (hand_count(Card::TateLiza) > 0 && play_tate_draw()) return;
    if (need_active_vstar() && play_tate_switch()) return;

    // Lusamine is a delayed recovery line and must not preempt a live direct connector.
    if (play_lusamine()) return;
    play_gladion();
  }

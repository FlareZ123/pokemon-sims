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
    // Arven remains a playable Supporter during Item lock and may choose Forest Seal
    // Stone as its Pokémon Tool target while selecting no Item: https://api.pokemontcg.io/v2/cards/sv1-166
    // Pokémon Tools are separate from Item cards after the Tool erratum, so that Tool
    // route remains legal during Item lock: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    const bool arven_fss_vstar_axis = need_vstar() && !state_.vstar_power_used &&
        might_be_unseen(Card::ForestSealStone) &&
        ((state_.active && state_.active->card == Card::RegidragoV && state_.active->tool == Tool::None) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) {
           return p.card == Card::RegidragoV && p.tool == Tool::None;
         }));

    if (known_gladion_axis) recover_once(Card::Gladion);
    if (arven_vstar_axis || arven_regi_axis || arven_fss_vstar_axis) recover_once(Card::Arven);
    if (need_active_vstar()) recover_once(Card::TateLiza);
    if (need_energy() && crispin_can_advance_energy_axis()) recover_once(Card::Crispin);
    if (need_payload()) recover_once(Card::ProfessorBurnet);
    if (arven_energy_axis || arven_payload_axis) recover_once(Card::Arven);

    if (targets.empty()) return targets;

    // These are the remaining modeled Supporter and Stadium cards, ordered from
    // least direct setup value to the connectors retained above. The list contains
    // only legal Lusamine targets and may select a second copy where available.
    for (const Card fallback : {Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                                Card::RoseannesBackup, Card::ProfessorTuro, Card::Channeler,
                                Card::Guzma, Card::Serena, Card::TateLiza, Card::StevensResolve,
                                Card::Arven, Card::Crispin, Card::ProfessorBurnet, Card::Gladion}) {
      recover_once(fallback);
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

  bool has_payable_roseanne_vstar_search() {
    if (item_locked()) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    if (!remove_one(state_.hand, Card::RoseannesBackup)) return false;

    // Evolution Incense searches an Evolution Pokémon without a discard cost:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    if (hand_count(Card::EvolutionIncense) > 0) {
      restore_hand();
      return true;
    }

    // Mysterious Treasure requires another hand card before it searches a Dragon
    // Pokémon. Test that cost after the Supporter has left hand:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    const bool permit_payload = can_play_payload_this_turn();
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload, true, true, Card::MysteriousTreasure).has_value()) {
      restore_hand();
      return true;
    }

    // Ultra Ball needs two other cards after Roseanne's Backup is played:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (hand_count(Card::UltraBall) > 0) {
      remove_one(state_.hand, Card::UltraBall);
      const auto first = choose_discard(permit_payload, true);
      if (first && remove_one(state_.hand, *first)) {
        const auto second = choose_discard(permit_payload && !is_payload(*first), true);
        if (second) {
          restore_hand();
          return true;
        }
      }
    }

    restore_hand();
    return false;
  }

  bool play_roseanne_vstar_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 || !need_vstar() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        count_of(state_.discard, Card::RegidragoVstar) == 0) {
      return false;
    }

    // Use the recovery only after legal knowledge proves no VSTAR remains in deck and
    // a currently payable Item can search the restored Evolution this turn.
    if (might_be_unseen(Card::RegidragoVstar) || !has_payable_roseanne_vstar_search()) return false;

    remove_one(state_.hand, Card::RoseannesBackup);
    state_.discard.push_back(Card::RoseannesBackup);
    state_.supporter_used = true;

    // Roseanne's Backup may choose its Pokémon mode and shuffle one Pokémon from
    // discard into the deck: https://api.pokemontcg.io/v2/cards/swsh9-148
    if (!remove_one(state_.discard, Card::RegidragoVstar)) {
      throw std::logic_error("Roseanne's Backup VSTAR target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(),
                                   Card::RegidragoVstar);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(Card::RegidragoVstar);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Roseanne's Backup shuffled Regidrago VSTAR from discard into the deck for a payable same-turn search.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;

    // Tate & Liza switch mode can repair the Active axis before a remaining Item
    // payload route resolves later in the same turn. Pick switch mode over draw
    // mode when Brilliant Blender already supplies the current-turn payload:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool tate_switch_only_active_axis = need_active_vstar() && !need_energy() && !need_payload();
    const bool tate_switch_then_blender_payload = need_active_vstar() && !can_free_retreat_with_latias() &&
        !need_energy() && need_payload() && has_live_blender_payload_line();
    if ((tate_switch_only_active_axis || tate_switch_then_blender_payload) && play_tate_switch()) return;

    // Exact Serena text: discard up to 3 cards, but "You must discard at least 1 card."
    // A zero-discard draw is illegal: https://api.pokemontcg.io/v2/cards/swsh12-164
    // This route requires an in-hand payload as that mandatory discard, then resolves
    // Serena before Prize recovery spends the only Supporter play.
    const bool serena_completes_only_missing_axis = need_payload() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_active_vstar() && can_play_payload_this_turn() &&
        hand_count(Card::Serena) > 0 && std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (serena_completes_only_missing_axis && play_serena()) return;

    // Roseanne's Backup can restore a known discarded Regidrago VSTAR before a
    // payable Item searches and evolves it in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (play_roseanne_vstar_recovery()) return;

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

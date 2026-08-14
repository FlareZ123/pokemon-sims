  bool issue_3545_steven_powerglass_treasure_route_available() const {
    // A Battle Compressor Energy seed plus Powerglass can convert Steven's third
    // unrestricted search into the ready-turn payload outlet instead of a second
    // Supporter. After Steven reserves VSTAR + Crispin + Mysterious Treasure,
    // Powerglass attaches the discarded Grass at end of T1. On T2 Crispin attaches
    // the second Grass, then Treasure discards the held Dragon after the attacker is
    // ready, satisfying the shared strict/flex same-turn JIT contract without
    // spending T2's Supporter action on Professor Burnet.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Same-ready-turn JIT / earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
    if (!prizes_known() || !strict_payload_timing() || !supporter_allowed() ||
        state_.turn != 1 || scenario_.going_first || state_.turn >= scenario_.max_turn ||
        item_locked_on_turn(state_.turn + 1) || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->double_dragon > 0 || state_.active->grass != 0 ||
        state_.active->fire < 1 || state_.active->tool != Tool::Powerglass ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 || hand_count(Card::Crispin) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 || !need_vstar() ||
        !need_energy() || !need_payload() || count_of(state_.discard, Card::Grass) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    if (deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    bool treasure_target_remains = false;
    for (const Card card : state_.deck) {
      if (!is_dragon_or_psychic(card)) continue;
      const int reserved = card == Card::RegidragoVstar ? 1 : 0;
      if (deck_count_after_search_started(card) > reserved) {
        treasure_target_remains = true;
        break;
      }
    }
    return treasure_target_remains;
  }

  bool play_steven_issue3545_powerglass_treasure() {
    if (!issue_3545_steven_powerglass_treasure_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-3545 Powerglass route");

    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::Crispin) ||
        !move_deck_to_hand(Card::MysteriousTreasure)) {
      throw std::logic_error("Issue-3545 Steven Powerglass targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-JIT-01; P-COMPRESS-01; P-KNOWLEDGE-01",
          "Searched BC-Powerglass route: Regidrago VSTAR, Crispin, Mysterious Treasure. Turn ended.");
    return true;
  }

  void choose_supporter_issue3545_base() {
    if (play_steven_issue3545_powerglass_treasure()) return;
    choose_supporter();
  }

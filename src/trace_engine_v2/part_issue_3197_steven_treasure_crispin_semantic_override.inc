  bool issue_3197_steven_treasure_crispin_route_available() const {
    // This route is evaluated only from K1/public state. Steven ends the current
    // turn after reserving VSTAR, Crispin, and one Grass. On the following turn,
    // Crispin supplies the other Grass axis while held Mysterious Treasure pays
    // its discard cost with the held Dragon payload on the actual ready turn.
    // StrictJit and MatchupFlexJit share that same-ready-turn payload contract;
    // Rule Box Ability lock does not suppress these Trainer actions.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, evolution, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Turn-two Item lock persists from the player's second turn onward: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // K1, JIT, lock, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3197
    if (!prizes_known() || !strict_payload_timing() || !supporter_allowed() ||
        held_arven_fss_blender_finish_available() ||
        item_locked_on_turn(state_.turn + 1) || // Persistent projected Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
        state_.turn + 1 > scenario_.max_turn || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn > state_.turn ||
        state_.active->double_dragon > 0 || state_.active->grass != 0 ||
        state_.active->fire < 1 || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Crispin) > 0 || hand_count(Card::Grass) > 0 ||
        hand_count(Card::BrilliantBlender) > 0 ||
        hand_count(Card::ProfessorBurnet) > 0 || !need_energy() ||
        !need_payload() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const int vstar_in_deck = deck_count_after_search_started(Card::RegidragoVstar);
    const int crispin_in_deck = deck_count_after_search_started(Card::Crispin);
    const int grass_in_deck = deck_count_after_search_started(Card::Grass);
    const int fire_in_deck = deck_count_after_search_started(Card::Fire);
    const bool treasure_target_remains = std::any_of(
        state_.deck.begin(), state_.deck.end(), [this](const Card card) {
          if (!is_dragon_or_psychic(card)) return false;
          const int reserved = card == Card::RegidragoVstar ? 1 : 0;
          return deck_count_after_search_started(card) > reserved;
        });

    return vstar_in_deck > 0 && crispin_in_deck > 0 && grass_in_deck >= 2 &&
        fire_in_deck > 0 && treasure_target_remains;
  }

  bool should_play_steven() const {
    return issue_3197_steven_treasure_crispin_route_available() ||
        should_play_steven_issue3197_original();
  }

  bool play_steven_issue3653_original() {
    if (!issue_3197_steven_treasure_crispin_route_available()) {
      return play_steven_issue3197_original();
    }

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::Crispin) ||
        !move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-3197 known Steven targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven may search up to three unrestricted cards. The held Treasure already
    // owns the ready-turn payload channel, so Blender/Burnet are connector-dominated
    // here and the complete target package is VSTAR + Crispin + Grass.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Same-ready-turn JIT / route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3197
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the semantic Treasure-Crispin route: Regidrago VSTAR, Crispin, Grass Energy. Turn ended.");
    return true;
  }

  bool play_issue_3653_steven_blender_package() {
    if (!issue_3222_steven_blender_fallback_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-3653 Blender package");

    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::BrilliantBlender)) {
      throw std::logic_error("Issue-3653 known Steven targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven may search up to three unrestricted cards, so reserve the known
    // VSTAR + Blender package without paying Quick Ball's discard cost next turn.
    // Blender can then put the Dragon payload into discard on the same turn that
    // the held Basic Energy completes Apex Dragon. This preserves the held Dragon
    // and Quick Ball when their paid connector route is dominated.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, search, discard, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1, same-ready-turn JIT, and connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3653
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the semantic VSTAR-Blender package: Regidrago VSTAR, Brilliant Blender. Turn ended.");
    return true;
  }

  bool play_steven() {
    if (issue_3197_steven_treasure_crispin_route_available()) {
      return play_steven_issue3653_original();
    }
    if (play_issue_3653_steven_blender_package()) return true;
    return play_steven_issue3653_original();
  }

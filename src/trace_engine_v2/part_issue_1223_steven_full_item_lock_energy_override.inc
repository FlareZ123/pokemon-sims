  bool issue_1223_full_item_lock_energy_package_available() const {
    const bool fss_available = hand_count(Card::ForestSealStone) > 0 ||
        deck_count_after_search_started(Card::ForestSealStone) > 0;
    const bool latias_available = hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) > 0;

    // Steven's Resolve may search any three cards. Under permanent Item lock,
    // a held Regidrago VSTAR makes another VSTAR redundant. Regidrago V,
    // Crispin, and Grass Energy establish the legal two-turn GGF attachment
    // schedule while held Professor Burnet preserves the strict-JIT payload:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Forest Seal Stone / Latias ex / Burnet: https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv8-76 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Official attachment and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, lock, and route priority specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1223
    return deck_seen_ && supporter_allowed() && !state_.turn_ended &&
        scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::FullItem && !scenario_.going_first &&
        scenario_.max_turn >= 4 && state_.turn == 1 && need_regi() &&
        state_.active && is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV && bench_space() >= 2 &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::Crispin) == 0 &&
        hand_count(Card::Grass) == 0 && hand_count(Card::Fire) == 0 &&
        deck_count_after_search_started(Card::RegidragoV) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        fss_available && latias_available && !in_play(Card::LatiasEx) &&
        payload_might_be_in_deck() &&
        ability_available_for_pokemon(Card::LatiasEx);
  }

  bool play_issue_1223_full_item_lock_energy_package() {
    if (!issue_1223_full_item_lock_energy_package_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    for (const Card target : {Card::RegidragoV, Card::Crispin, Card::Grass}) {
      if (!move_deck_to_hand(target)) {
        throw std::logic_error("Issue 1223 Steven target disappeared after K1 validation");
      }
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy. Turn ended.");
    return true;
  }

  bool play_steven() {
    if (play_issue_1223_full_item_lock_energy_package()) return true;
    return play_steven_issue1223_original();
  }

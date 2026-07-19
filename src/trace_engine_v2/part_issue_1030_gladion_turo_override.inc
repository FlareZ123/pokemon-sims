  bool held_payload_completes_current_turn_without_gladion() const {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0 ||
        !prizes_known() || !need_payload() ||
        !can_play_payload_this_turn() ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        !prize_payload_one_discard_outlet_has_legal_target()) {
      return false;
    }

    // Project the rest of the exact public turn after reserving the Supporter play.
    // A held permitted Dragon must pay a legally targeted one-discard Item and the
    // remaining held resources must still complete Active VSTAR, GGF, and payload:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1056
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    projected.state_.supporter_used = true;
    projected.run_turn();

    return projected.active_is_vstar() && projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  bool known_prized_mysterious_treasure_completes_current_turn() const {
    if (!supporter_allowed() || hand_count(Card::Gladion) == 0 ||
        !prizes_known() || item_locked() ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        prize_count_after_reveal(Card::MysteriousTreasure) == 0 ||
        prize_count_after_reveal(Card::RegidragoVstar) > 0 ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        !need_vstar() || !need_payload() || !can_play_payload_this_turn() ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // K1 permits the known Prize exchange to be projected without reading hidden
    // information. The recovered Item must pay its printed one-card cost with a held
    // Dragon, search the known deck-resident VSTAR, and finish every axis this turn:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-current-turn policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1059
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;

    const auto held_gladion = std::find(projected.state_.hand.begin(),
                                        projected.state_.hand.end(), Card::Gladion);
    const auto prized_treasure = std::find(projected.state_.prizes.begin(),
                                           projected.state_.prizes.end(),
                                           Card::MysteriousTreasure);
    if (held_gladion == projected.state_.hand.end() ||
        prized_treasure == projected.state_.prizes.end()) {
      return false;
    }

    projected.state_.hand.erase(held_gladion);
    projected.state_.supporter_used = true;
    projected.prizes_revealed_ = true;
    *prized_treasure = Card::Gladion;
    projected.state_.hand.push_back(Card::MysteriousTreasure);
    projected.shuffle(projected.state_.prizes);
    projected.run_turn();

    const bool treasure_resolved = std::find(
        projected.state_.discard.begin(), projected.state_.discard.end(),
        Card::MysteriousTreasure) != projected.state_.discard.end();
    const bool payload_entered_discard = std::any_of(
        projected.state_.discarded_this_turn.begin(),
        projected.state_.discarded_this_turn.end(), is_payload);
    return treasure_resolved && payload_entered_discard &&
        projected.active_is_vstar() && projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  bool known_gladion_turo_t4_promotion_route() const {
    if (!prizes_known() || state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 4 ||
        !state_.active || state_.active->card != Card::DialgaGX ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        prize_count_after_reveal(Card::ProfessorTuro) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    return std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn == state_.turn && pokemon.fire >= 1;
        });
  }

  bool play_gladion() {
    if (held_payload_completes_current_turn_without_gladion()) {
      // The held Dragon and legal one-discard Item already reach the same earliest
      // ready state. Preserve Gladion, the Supporter play, and Prize configuration:
      // https://api.pokemontcg.io/v2/cards/sm4-95
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://github.com/FlareZ123/pokemon-sims/issues/1056
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1056-held-live-payload", state_.turn,
            "HOLD SUPPORTER", "R-GLADION-01; P-DCI-01; P-COMPRESS-01",
            "Retained Gladion because a held Dragon and legal one-discard Item already complete the current-turn ready route.");
      }
      return false;
    }

    if (known_prized_mysterious_treasure_completes_current_turn()) {
      // Prefer the known prized Item that completes VSTAR and strict-JIT payload now
      // over a prized Arven route that cannot resolve until the following turn:
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/1059
      remove_one(state_.hand, Card::Gladion);
      state_.supporter_used = true;
      prizes_revealed_ = true;
      const auto prize = std::find(state_.prizes.begin(), state_.prizes.end(),
                                   Card::MysteriousTreasure);
      if (prize == state_.prizes.end()) {
        throw std::logic_error("Known prized Mysterious Treasure disappeared");
      }
      *prize = Card::Gladion;
      state_.hand.push_back(Card::MysteriousTreasure);
      shuffle(state_.prizes);
      trace("PLAY SUPPORTER",
            "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
            "Exchanged Gladion for Mysterious Treasure to complete the current-turn VSTAR and payload route.");
      return true;
    }

    if (!known_gladion_turo_t4_promotion_route()) {
      return play_gladion_issue1030_original();
    }

    // K1 proves this complete T4 line from cards already held or deterministically
    // searchable. Take prized Professor Turo instead of a redundant second Basic,
    // use held Crispin for GF, Steven for VSTAR plus the final Grass, Quick Ball for
    // the strict-JIT Dragon discard, then Turo the attachment-free Dialga-GX Active:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, attachment, evolution, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1030
    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto prize = std::find(state_.prizes.begin(), state_.prizes.end(),
                                 Card::ProfessorTuro);
    if (prize == state_.prizes.end()) {
      throw std::logic_error("Known prized Professor Turo disappeared");
    }
    *prize = Card::Gladion;
    state_.hand.push_back(Card::ProfessorTuro);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER",
          "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Looked at Prize cards and exchanged Gladion for Professor Turo's Scenario.");
    return true;
  }

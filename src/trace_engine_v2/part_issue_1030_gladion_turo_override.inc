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

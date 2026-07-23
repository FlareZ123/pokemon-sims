  bool known_burnet_t3_steven_route_available() const {
    if (!deck_seen_ || state_.turn != 2 || scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit || scenario_.max_turn < 3 ||
        !supporter_allowed() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::ProfessorBurnet) > 0 ||
        hand_count(Card::BrilliantBlender) > 0 ||
        !need_vstar() || !need_payload()) {
      return false;
    }

    // At K1, Steven can reserve the known VSTAR and Professor Burnet, then end T2.
    // On T3 the prior-turn Active evolves and Burnet searches and discards the
    // permitted Dragon payload during the strict-JIT ready turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core evolution, Supporter, search, discard, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // This route uses no Item or Rule Box Pokémon Ability, so Item, Rule Box
    // Ability, and combined locks do not suppress Steven, ordinary evolution,
    // or Professor Burnet: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1108 https://github.com/FlareZ123/pokemon-sims/issues/1186
    return deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        payload_might_be_in_deck();
  }

  bool play_known_burnet_t3_steven_route() {
    if (!known_burnet_t3_steven_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::ProfessorBurnet)) {
      throw std::logic_error("Known Burnet T3 Steven targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven searches up to three unrestricted cards. Taking only VSTAR and Burnet
    // preserves the remaining deck payload for Burnet's next-turn search:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://github.com/FlareZ123/pokemon-sims/issues/1108
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the known T3 Burnet route: Regidrago VSTAR, Professor Burnet. Turn ended.");
    return true;
  }

  bool known_turo_t4_steven_route_available() const {
    if (!deck_seen_ || state_.turn != 3 || scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 4 ||
        !supporter_allowed() || !state_.active ||
        state_.active->card != Card::DialgaGX ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::ProfessorTuro) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool prepared_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn &&
              pokemon.grass >= 1 && pokemon.fire >= 1;
        });
    if (!prepared_regidrago) return false;

    // Steven can reserve the missing VSTAR and final Grass, then ends T3. On T4 the
    // prior-turn Regidrago evolves, the manual attachment completes GGF, Quick Ball
    // discards the held Dragon during the strict-JIT ready turn, and the already held
    // Professor Turo removes the Basic Active so the completed VSTAR is promoted:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Core evolution, attachment, Supporter, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1030
    return hand_count(Card::RegidragoVstar) +
                deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Grass) +
                deck_count_after_search_started(Card::Grass) > 0;
  }

  bool play_known_turo_t4_steven_route() {
    if (!known_turo_t4_steven_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> found;
    if (hand_count(Card::RegidragoVstar) == 0) {
      if (!move_deck_to_hand(Card::RegidragoVstar)) {
        throw std::logic_error("Known Turo route VSTAR target disappeared");
      }
      found.push_back(Card::RegidragoVstar);
    }
    const Pokemon* target = target_regi();
    if (target != nullptr && target->grass < 2 && hand_count(Card::Grass) == 0) {
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("Known Turo route Grass target disappeared");
      }
      found.push_back(Card::Grass);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven searches up to three cards, so selecting only the missing Evolution and
    // Energy pieces preserves the held Quick Ball, Dragon payload, and Turo promotion:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://github.com/FlareZ123/pokemon-sims/issues/1030
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the known T4 Turo promotion route: " + join_cards(found) +
              ". Turn ended.");
    return true;
  }

  bool issue_1223_full_item_lock_steven_grass_route_available() const {
    // The proven Steven package is determined by public K1 axes, not by Item lock.
    // Admit the same fully guarded target split when no lock is present, while all
    // other lock models retain their existing selectors:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Original full-Item-lock route: https://github.com/FlareZ123/pokemon-sims/issues/1223
    // Confirmed no-lock bug: https://github.com/FlareZ123/pokemon-sims/issues/1378
    const bool supported_lock_state = scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::None;
    if (!deck_seen_ || !supporter_allowed() ||
        scenario_.dci != DciProfile::StrictJit || !supported_lock_state ||
        state_.turn + 2 > scenario_.max_turn || !state_.active ||
        !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::RegidragoV) > 0 ||
        hand_count(Card::Crispin) > 0 ||
        hand_count(Card::Grass) > 0 ||
        in_play(Card::RegidragoV) || in_play(Card::RegidragoVstar) ||
        in_play(Card::LatiasEx) || bench_space() < 2 ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }

    // Steven's Resolve may search any three cards. With VSTAR and Burnet already
    // held under permanent Item lock, the deterministic package is Regidrago V,
    // Crispin, and Grass Energy. A second VSTAR duplicates a completed axis, while
    // the searched Grass preserves the manual-attachment plus Crispin GGF schedule:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official search, attachment, evolution, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, lock, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1223
    const bool latias_available =
        hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) > 0;
    const bool stone_available =
        hand_count(Card::ForestSealStone) > 0 ||
        deck_count_after_search_started(Card::ForestSealStone) > 0;
    return deck_count_after_search_started(Card::RegidragoV) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        latias_available && stone_available && payload_might_be_in_deck();
  }

  bool should_play_steven() const {
    return should_play_steven_issue1030_original() ||
        known_burnet_t3_steven_route_available() ||
        known_turo_t4_steven_route_available();
  }

  bool play_steven() {
    // A held Arven can complete the current turn through Forest Seal Stone and
    // Brilliant Blender, so do not enter any Steven route that ends the turn:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1067
    if (held_arven_fss_blender_finish_available()) return false;
    if (play_known_burnet_t3_steven_route()) return true;
    if (play_known_turo_t4_steven_route()) return true;

    const bool issue_1223_route =
        issue_1223_full_item_lock_steven_grass_route_available();
    const int vstar_before = hand_count(Card::RegidragoVstar);
    const int grass_before = hand_count(Card::Grass);
    const int regi_before = hand_count(Card::RegidragoV);
    const int crispin_before = hand_count(Card::Crispin);

    const bool played = play_steven_issue1030_original();
    if (!played || !issue_1223_route) return played;

    const int vstar_found = hand_count(Card::RegidragoVstar) - vstar_before;
    const int grass_found = hand_count(Card::Grass) - grass_before;
    const int regi_found = hand_count(Card::RegidragoV) - regi_before;
    const int crispin_found = hand_count(Card::Crispin) - crispin_before;
    auto grass_in_deck =
        std::find(state_.deck.begin(), state_.deck.end(), Card::Grass);

    if (vstar_found != 1 || grass_found != 0 || regi_found != 1 ||
        crispin_found != 1 || grass_in_deck == state_.deck.end()) {
      return true;
    }

    // The delegated Steven search already randomized the remaining deck. Replace only
    // the redundant selected VSTAR with one known Grass in the same randomized slot,
    // preserving deck size, order, and every other selected card:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Grass Energy and one manual attachment per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route specification: https://github.com/FlareZ123/pokemon-sims/issues/1223
    if (!remove_one(state_.hand, Card::RegidragoVstar)) {
      throw std::logic_error("Redundant Steven VSTAR target disappeared");
    }
    *grass_in_deck = Card::RegidragoVstar;
    state_.hand.push_back(Card::Grass);

    if (trace_ != nullptr && trace_->enabled && !trace_->lines.empty() &&
        trace_->lines.back().find("PLAY SUPPORTER") != std::string::npos &&
        trace_->lines.back().find("Searched up to 3 cards") != std::string::npos) {
      trace_->lines.pop_back();
    }
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy. Turn ended.");
    return true;
  }

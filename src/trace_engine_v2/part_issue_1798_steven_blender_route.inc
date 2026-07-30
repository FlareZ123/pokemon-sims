  int issue_1798_blender_turn_{0};

  int issue_1798_deck_payload_count() const {
    int count = 0;
    for (const Card payload : {Card::Appletun, Card::MegaDragonite,
                               Card::Dragapult, Card::GoodraVstar,
                               Card::DialgaGX}) {
      count += deck_count_after_search_started(payload);
    }
    return count;
  }

  bool issue_1798_steven_blender_route_available() const {
    const bool held_direct_vstar_connector =
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::EvolutionIncense) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::UltraBall) > 0 ||
        hand_count(Card::PokemonCommunication) > 0 ||
        hand_count(Card::ForestSealStone) > 0 ||
        hand_count(Card::Arven) > 0 || hand_count(Card::SecretBox) > 0;

    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        state_.turn + 1 > scenario_.max_turn || state_.turn_ended ||
        !prizes_known() || !supporter_allowed() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 1 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        hand_count(Card::Fire) == 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        held_direct_vstar_connector ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        issue_1798_deck_payload_count() == 0) {
      return false;
    }

    // K1 proves the complete two-turn route before the T3 attachment is spent.
    // Fire is surplus by type, yet required as the third total attachment before
    // Steven ends T3. Steven banks Regidrago VSTAR and Grass; on T4 the prior-turn
    // Active evolves, Grass reaches GGFF, and Brilliant Blender discards a permitted
    // Dragon from the deck during the strict-JIT ready turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, attachment, evolution, Item, search, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1798
    return true;
  }

  bool start_issue_1798_steven_blender_route() {
    if (!issue_1798_steven_blender_route_available()) return false;

    if (!remove_one(state_.hand, Card::Fire)) {
      throw std::logic_error("Issue-1798 held Fire disappeared");
    }
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-STEVEN-01; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Fire Energy manually to the Active Regidrago V for the deterministic T4 Steven and Blender route.");

    if (!remove_one(state_.hand, Card::StevensResolve)) {
      throw std::logic_error("Issue-1798 Steven's Resolve disappeared");
    }
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1798 route");

    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("Issue-1798 Regidrago VSTAR target disappeared");
    }
    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-1798 Grass Energy target disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    issue_1798_blender_turn_ = state_.turn + 1;

    // Steven may search up to three cards and ends the turn. Searching only the two
    // missing pieces preserves held Brilliant Blender and every deck payload for T4:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official deck-search and end-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1798
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched Regidrago VSTAR and Grass Energy for the committed T4 Brilliant Blender finish. Turn ended.");
    return true;
  }

  bool issue_1798_blender_finish_available() const {
    return issue_1798_blender_turn_ == state_.turn &&
        scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && !state_.turn_ended &&
        prizes_known() && !state_.manual_energy_used && !item_locked() &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 2 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Grass) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 &&
        issue_1798_deck_payload_count() > 0;
  }

  bool complete_issue_1798_steven_blender_route() {
    if (issue_1798_blender_turn_ != state_.turn) return false;
    if (!issue_1798_blender_finish_available()) {
      issue_1798_blender_turn_ = 0;
      return false;
    }

    evolve_best_regi();
    if (!active_is_vstar()) {
      throw std::logic_error("Issue-1798 Active Regidrago V did not evolve");
    }
    if (!remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue-1798 searched Grass Energy disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-RVS-01; P-COMPRESS-01",
          "Grass Energy manually to the Active Regidrago VSTAR, reaching GGFF.");

    if (!remove_one(state_.hand, Card::BrilliantBlender)) {
      throw std::logic_error("Issue-1798 Brilliant Blender disappeared");
    }
    state_.discard.push_back(Card::BrilliantBlender);
    std::optional<Card> discarded_payload;
    for (const Card payload : {Card::Appletun, Card::MegaDragonite,
                               Card::Dragapult, Card::GoodraVstar,
                               Card::DialgaGX}) {
      if (move_deck_to_discard(payload)) {
        discarded_payload = payload;
        break;
      }
    }
    if (!discarded_payload) {
      throw std::logic_error("Issue-1798 K1-proven Blender payload disappeared");
    }
    shuffle(state_.deck);
    outcome_.used_blender = true;
    trace("PLAY ITEM", "R-BLENDER-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-JIT-01",
          "Brilliant Blender searched and discarded the K1-proven deck payload: " +
              std::string(name(*discarded_payload)) + ".");
    issue_1798_blender_turn_ = 0;

    // Brilliant Blender searches the deck, so the held Dragon is intentionally
    // preserved. A K1-proven deck Dragon enters discard on T4 after the Active has
    // evolved and reached GGFF, satisfying Apex Dragon's GGF cost and strict JIT:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, attachment, Item, search, discard, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Strict-JIT and success horizon: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1798
    return state_.active->grass >= 2 && state_.active->fire >= 1 &&
        payload_ready();
  }

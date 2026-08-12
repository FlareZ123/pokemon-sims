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

    // The bank/finish packet is relative to the current public state. Steven is a
    // current-turn Supporter and Brilliant Blender is an Item on the recorded next
    // turn, so use their actual legality rather than the historical seat/T3/no-lock
    // witness. Rule Box Ability lock leaves both Trainer actions legal.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Supporter, Item, search, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Earliest-route objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route / cross-profile / semantic-admission bugs: https://github.com/FlareZ123/pokemon-sims/issues/1798 https://github.com/FlareZ123/pokemon-sims/issues/2764 https://github.com/FlareZ123/pokemon-sims/issues/3279
    if (!strict_payload_timing() ||
        item_locked_on_turn(state_.turn + 1) ||
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

    // K1 proves the complete two-turn route before the current attachment is spent.
    // Fire is surplus by type, yet required as the third total attachment before
    // Steven ends the bank turn. Steven banks Regidrago VSTAR and Grass; on the next
    // turn the prior-turn Active evolves, Grass completes the Energy axis, and held
    // Brilliant Blender discards a permitted Dragon during that same JIT-ready turn.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, attachment, evolution, Item, search, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, same-turn JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1798
    // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2764
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3279
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
          "Fire Energy manually to the Active Regidrago V for the deterministic T" +
              std::to_string(state_.turn + 1) + " Steven and Blender route.");

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
    // missing pieces preserves held Brilliant Blender and every deck payload for the
    // recorded following turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official deck-search and end-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Original confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1798
    // Semantic-admission follow-up: https://github.com/FlareZ123/pokemon-sims/issues/3279
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched Regidrago VSTAR and Grass Energy for the committed T" +
              std::to_string(issue_1798_blender_turn_) +
              " Brilliant Blender finish. Turn ended.");
    return true;
  }

  bool issue_1798_blender_finish_available() const {
    // The committed finish is governed by the recorded relative turn and current
    // Item legality. Rule Box Ability lock does not suppress evolution, attachment,
    // or Brilliant Blender, so no seat or blanket lock identity belongs here.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Same-ready-turn JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3279
    return issue_1798_blender_turn_ == state_.turn && strict_payload_timing() &&
        !state_.turn_ended && prizes_known() && !state_.manual_energy_used &&
        !item_locked() && state_.active &&
        state_.active->card == Card::RegidragoV &&
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
          "Grass Energy manually to the Active Regidrago VSTAR, completing the Apex Energy requirement.");

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
    // preserved. A K1-proven deck Dragon enters discard after the Active has evolved
    // and completed its Energy requirement, satisfying Apex Dragon and same-turn JIT:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, attachment, Item, search, discard, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Same-turn JIT and success horizon: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
    // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2764
    // Semantic-admission follow-up: https://github.com/FlareZ123/pokemon-sims/issues/3279
    return pays_apex_energy_cost(*state_.active) && payload_ready();  // DDE/Apex energy semantics: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ https://api.pokemontcg.io/v2/cards/swsh12-136 https://github.com/FlareZ123/pokemon-sims/issues/2368
  }

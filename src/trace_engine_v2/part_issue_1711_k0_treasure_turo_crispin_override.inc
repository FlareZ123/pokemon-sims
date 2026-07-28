  bool issue_1711_k0_treasure_turo_crispin_available() const {
    const bool clean_active_tapu = state_.active &&
        state_.active->card == Card::TapuLeleGX &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        state_.active->tool == Tool::None;

    // This K0 decision uses only the visible hand and board, registered recipe
    // counts, an open Bench, and plausible fixed-list targets. Mysterious Treasure
    // creates K1 during resolution before the continuation reads exact deck state:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // K0/K1 and route-aware DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1711
    return secret_box_combo_enabled() &&
        scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && !scenario_.going_first &&
        state_.turn == 2 && state_.turn < scenario_.max_turn &&
        !deck_seen_ && !prizes_revealed_ && !item_locked() &&
        supporter_allowed() && !state_.manual_energy_used &&
        !state_.turn_ended && clean_active_tapu && state_.bench.empty() &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::ProfessorTuro) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::Grass) > 0 &&
        might_be_unseen(Card::RegidragoV) &&
        might_be_unseen(Card::Crispin) &&
        might_be_unseen(Card::Grass) &&
        might_be_unseen(Card::Fire) &&
        issue_1605_redundant_payload_cost().has_value();
  }

  bool play_issue_1711_k0_treasure_turo_crispin_route() {
    if (!issue_1711_k0_treasure_turo_crispin_available()) return false;
    const Card repeated_payload = *issue_1605_redundant_payload_cost();

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(repeated_payload,
                           "Mysterious Treasure issue-1711 repeated-payload cost",
                           "R-MT-01; P-DCI-01; P-KNOWLEDGE-01")) {
      throw std::logic_error("Issue-1711 repeated payload disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1711 K0 route");
    const bool found_regidrago = move_deck_to_hand(Card::RegidragoV);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-KNOWLEDGE-01",
          found_regidrago
              ? "Discarded one repeated Dragon payload and searched Regidrago V."
              : "Discarded one repeated Dragon payload; no Regidrago V remained in deck.");
    if (!found_regidrago) return false;

    if (!bench_from_hand(Card::RegidragoV, false)) {
      throw std::logic_error("Issue-1711 Regidrago Bench action disappeared");
    }

    const bool complete_k1_continuation =
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        hand_count(Card::ProfessorTuro) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::Grass) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (!complete_k1_continuation) return false;

    // K1 now proves the entire continuation. Turo frees the Active position and
    // returns Tapu Lele-GX, replaying Tapu legally triggers Wonder Tag for Crispin.
    // The held Grass attachment establishes the prior-turn Regidrago line, while
    // Crispin plus the next manual attachment completes GGF on T3:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Bench, Ability, attachment, promotion, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1711
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;
    state_.hand.push_back(Card::TapuLeleGX);
    state_.active = state_.bench.front();
    state_.bench.erase(state_.bench.begin());
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; P-COMPRESS-01",
          "Professor Turo returned Active Tapu Lele-GX and promoted Regidrago V.");

    if (!bench_from_hand(Card::TapuLeleGX, false)) {
      throw std::logic_error("Issue-1711 Tapu replay failed");
    }
    record_deck_search_knowledge("Wonder Tag issue-1711 route");
    if (!move_deck_to_hand(Card::Crispin)) {
      throw std::logic_error("Issue-1711 Crispin disappeared after K1 proof");
    }
    shuffle(state_.deck);
    trace("USE ABILITY", "R-TAPU-01; R-CRISPIN-01; P-KNOWLEDGE-01",
          "Replayed Tapu Lele-GX and used Wonder Tag to search Crispin.");

    if (!remove_one(state_.hand, Card::Grass) || !state_.active ||
        state_.active->card != Card::RegidragoV) {
      throw std::logic_error("Issue-1711 Grass attachment target disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; P-COMPRESS-01",
          "Grass Energy manually to Regidrago V for the deterministic T3 route.");
    return true;
  }

  void run_secret_box_turn_issue1711() {
    if (play_issue_1711_k0_treasure_turo_crispin_route()) {
      trace("POLICY", "R-MT-01; R-TAPU-01; R-CRISPIN-01; P-AXIS-01",
            "Completed the K0 Treasure, Turo, replayed Wonder Tag, and T3 Crispin setup: " +
                state_line());
      return;
    }
    run_secret_box_turn();
  }

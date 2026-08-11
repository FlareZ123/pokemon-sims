  bool issue_3152_complete_steven_t2_route_available() const {
    const bool registered_pineco_recipe =
        registered_recipe_count(Card::SecretBox) == 1 &&
        registered_recipe_count(Card::Pineco) == 2 &&
        registered_recipe_count(Card::ForretressEx) == 2;
    const bool next_turn_item_locked =
        scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        scenario_.locks == LockMode::TurnTwoItem;

    if (!registered_pineco_recipe || !secret_box_combo_enabled() ||
        !strict_payload_timing() || scenario_.going_first || state_.turn != 1 ||
        scenario_.max_turn < 2 || state_.turn_ended || !supporter_allowed() ||
        state_.supporter_used || next_turn_item_locked || !prizes_known() ||
        !known_prize(Card::SecretBox) || hand_count(Card::Gladion) == 0 ||
        hand_count(Card::StevensResolve) == 0 || !state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn != state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 0 ||
        hand_count(Card::Fire) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::Grass) <= 1 ||
        deck_count_after_search_started(Card::Fire) <= 1 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Steven removes Regidrago VSTAR from the future Treasure target pool. Require
    // two additional Psychic or Dragon targets so the mandatory T2 draw cannot
    // remove Treasure's final legal search target. The held Dragon payload remains
    // in hand to pay Treasure's printed discard cost on the ready turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Core search, Supporter, Item, attachment, evolution, and draw procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and equal-turn resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Secret Box/Pineco policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
    const int treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic));
    return treasure_targets >= 3;
  }

  bool play_issue_3152_complete_steven_t2_route() {
    if (!issue_3152_complete_steven_t2_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> selected;
    for (const Card card : {Card::RegidragoVstar, Card::Crispin,
                            Card::MysteriousTreasure}) {
      if (move_deck_to_hand(card)) selected.push_back(card);
    }
    if (selected.size() != 3U) {
      throw std::logic_error(
          "Issue 3152 Steven targets disappeared after proven K1 preflight");
    }
    shuffle(state_.deck);
    issue_1420_direct_t2_treasure_route_ = true;

    // This route reaches the same earliest T2 readiness as Gladion -> Secret Box
    // while preserving the known prized ACE SPEC, Gladion, and Secret Box's three
    // discard costs. The existing #1420 continuation resolves Crispin, the remaining
    // manual attachment, Regidrago VSTAR evolution, and same-turn Treasure payload:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Equal-turn resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-CRISPIN-01; R-MT-01; P-AXIS-01; P-DCI-01",
          "Steven's Resolve banked Regidrago VSTAR, Crispin, and Mysterious "
          "Treasure for the complete T2 route, preserving prized Secret Box "
          "and Gladion; the turn ended.");
    return true;
  }

  bool bench_pineco_if_useful_issue3152() {
    const bool benched = bench_pineco_if_useful();
    if (play_issue_3152_complete_steven_t2_route()) return true;
    return benched;
  }

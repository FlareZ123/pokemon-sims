  std::optional<Card> issue_2202_treasure_tapu_crispin_payload_cost() const {
    const bool route_available =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::Crispin) == 0 && bench_space() > 0 &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    if (!route_available) return std::nullopt;

    // Treasure's mandatory discard is the strict current-turn Apex Dragon outlet.
    // The same search finds Tapu Lele-GX, Wonder Tag finds Crispin, and Crispin
    // attaches the second Grass needed by the public K1 GF Active VSTAR.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Bench, Ability, Supporter, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, current-turn JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Reclaimed confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2202
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    const auto cost = issue_2202_treasure_tapu_crispin_payload_cost();
    if (!cost) return play_mysterious_treasure_issue2202_original(permit_payload);

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*cost, "Mysterious Treasure cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-2202 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-2202 Tapu route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("projected issue-2202 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          "Spent the current-turn Dragon payload and searched Tapu Lele-GX for Wonder Tag -> Crispin.");
    return true;
  }

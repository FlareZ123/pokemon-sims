  std::optional<Card> issue_2227_treasure_gg_tapu_crispin_payload_cost() const {
    const bool route_available =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        supporter_allowed() && active_is_vstar() &&
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        need_payload() && hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::Crispin) == 0 && bench_space() > 0 &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    if (!route_available) return std::nullopt;

    // Treasure's mandatory discard is the current-turn JIT outlet. In this
    // public K1 GG state, the same search finds Tapu Lele-GX, Wonder Tag finds
    // Crispin, and Crispin attaches the missing Fire to the Active VSTAR.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Bench, Ability, Supporter, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2227
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_2232_treasure_tapu_crispin_two_energy_cost() const {
    const bool complete_public_route =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 0 &&
        !state_.manual_energy_used && need_payload() && bench_space() > 0 &&
        !in_play(Card::TapuLeleGX) && hand_count(Card::TapuLeleGX) == 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    if (!complete_public_route) return std::nullopt;

    // Mysterious Treasure can spend a held Dragon while searching Tapu Lele-GX.
    // Wonder Tag then finds Crispin, which attaches one missing Basic Energy and
    // puts the other into hand for the unused manual attachment. The same Treasure
    // payment therefore establishes the strict current-turn payload and completes
    // the Active Regidrago VSTAR from G to GGF without consuming another route.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mega Dragonite ex and Hisuian Goodra VSTAR payload witnesses: https://api.pokemontcg.io/v2/cards/me2pt5-152 https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, discard, search, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2232
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_2229_treasure_vstar_unused_supporter_cost() const {
    const bool route_available =
        strict_payload_timing() && prizes_known() && !item_locked() &&
        !state_.supporter_used && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire >= 1 &&
        need_vstar() && need_payload() &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
    if (!route_available) return std::nullopt;

    // The prior-turn Regidrago V already has Apex Dragon's GGF cost. Mysterious
    // Treasure requires no Supporter action, so an unused Supporter slot does not
    // change this complete Item route: spend a held Dragon as Treasure's mandatory
    // cost, search Regidrago VSTAR, and evolve immediately for same-turn readiness.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Mega Dragonite ex payload witness: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2229
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1876_treasure_payload_cost() const {
    if (const auto cost = issue_2227_treasure_gg_tapu_crispin_payload_cost()) {
      return cost;
    }
    if (const auto cost = issue_2232_treasure_tapu_crispin_two_energy_cost()) {
      return cost;
    }
    if (const auto cost = issue_2229_treasure_vstar_unused_supporter_cost()) {
      return cost;
    }
    const bool complete_public_route =
        strict_payload_timing() && prizes_known() &&
        !item_locked() && supporter_allowed() && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::Crispin) > 0 &&
        crispin_can_advance_energy_axis() &&
        std::any_of(state_.deck.begin(), state_.deck.end(),
                    is_dragon_or_psychic);
    if (!complete_public_route) return std::nullopt;

    // Mysterious Treasure may discard any card. In this public K1 GF state,
    // held Crispin attaches the missing Grass, so a visible Dragon is DCI 1
    // because this exact cost creates the current-turn Apex Dragon payload.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Supplied card-data corpus: https://github.com/PokemonTCG/pokemon-tcg-data/blob/master/cards/en/sm6.json
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dialga-GX and Dragapult ex: https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Supporter, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1876 https://github.com/FlareZ123/pokemon-sims/issues/1896
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1869_quick_ball_burnet_cost() const {
    const bool complete_public_route =
        scenario_.dci == DciProfile::StrictJit && prizes_known() &&
        !item_locked() && supporter_allowed() && active_is_vstar() &&
        !need_energy() && need_payload() && hand_count(Card::QuickBall) > 0 &&
        bench_space() > 0 && !in_play(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        payload_might_be_in_deck();
    if (!complete_public_route) return std::nullopt;

    // Active GGF leaves only the current-turn payload axis. Quick Ball can spend a
    // visible Supporter with no remaining setup role, find Tapu Lele-GX, and use
    // Wonder Tag for Professor Burnet, which discards the Dragon payload this turn.
    // Full deck inspection or full Prize inspection establishes the same public K1
    // deck composition, so the route uses prizes_known() rather than provenance:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supplied card-data corpus: https://github.com/PokemonTCG/pokemon-tcg-data/tree/master/cards/en
    // Official Item, discard, search, Bench, Ability, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector composition, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1869
    for (const Card card : {Card::ProfessorTuro, Card::ErikasInvitation}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> choose_discard_issue1876(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    if (excluded_from_cost == Card::QuickBall) {
      if (const auto cost = issue_1869_quick_ball_burnet_cost()) return cost;
    }
    if (excluded_from_cost == Card::MysteriousTreasure) {
      if (const auto payload = issue_1876_treasure_payload_cost()) {
        return payload;
      }
    }
    return choose_discard_issue1740(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
  }

#undef choose_discard
#define choose_discard choose_discard_issue1876
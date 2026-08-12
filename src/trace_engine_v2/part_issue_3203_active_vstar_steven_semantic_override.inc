  bool issue_3203_active_vstar_steven_crispin_treasure_route_available() const {
    const bool route_lock_allows_items =
        scenario_.locks == LockMode::None ||
        scenario_.locks == LockMode::FullRuleBoxAbility;

    // The current public state, rather than the historical seat/T2 witness, owns
    // this continuation. Steven searches Crispin + Grass and ends the turn. On the
    // following turn Crispin supplies one Grass, the manual attachment supplies the
    // second, and held Mysterious Treasure discards the held Dragon on the exact
    // ready turn. StrictJit and MatchupFlexJit therefore share the route. Rule Box
    // Ability lock remains legal because every required continuation action here is
    // a Trainer, attachment, or ordinary attack-cost state transition.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, discard, search, attachment, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1/JIT/lock/priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
    if (!prizes_known() || !strict_payload_timing() || !route_lock_allows_items ||
        state_.turn + 1 > scenario_.max_turn || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        state_.active->grass != 0 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Grass) > 0 || hand_count(Card::Crispin) > 0 ||
        !need_energy() || !need_payload() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const int grass_in_deck = deck_count_after_search_started(Card::Grass);
    const int fire_in_deck = deck_count_after_search_started(Card::Fire);
    const int treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic));
    if (deck_count_after_search_started(Card::Crispin) == 0 ||
        grass_in_deck < 2 || fire_in_deck == 0 || treasure_targets == 0) {
      return false;
    }

    const int post_steven_deck_size = static_cast<int>(state_.deck.size()) - 2;
    const int grass_after_steven = grass_in_deck - 1;
    const int failing_topdecks = static_cast<int>(grass_after_steven == 1) +
        static_cast<int>(fire_in_deck == 1);

    // Steven removes Crispin and one Grass from the known deck. Preserve the legacy
    // topdeck-risk calculation because drawing the final copy of a required Basic
    // Energy type before Crispin resolves can collapse that next-turn line.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Route-risk provenance: https://github.com/FlareZ123/pokemon-sims/issues/1098
    // Semantic-admission correction: https://github.com/FlareZ123/pokemon-sims/issues/3203
    return post_steven_deck_size > failing_topdecks;
  }

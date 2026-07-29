  int issue_1811_route_turn_{-1};

  bool issue_1811_public_arven_route_candidate() const {
    // This narrow preflight uses only the public T1 state. Arven performs the real
    // legal deck inspection before the policy tests the exact route components:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, Tool, attachment, evolution, VSTAR Power, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1811
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && !scenario_.going_first &&
        state_.turn == 1 && scenario_.max_turn >= 2 && supporter_allowed() &&
        !item_locked() && !state_.vstar_power_used && state_.manual_energy_used &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn == 1 && state_.active->grass == 1 &&
        state_.active->fire == 0 && state_.active->tool == Tool::None &&
        hand_count(Card::Arven) > 0 && hand_count(Card::Fire) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::MysteriousTreasure) == 0 &&
        hand_count(Card::ForestSealStone) == 0 &&
        hand_count(Card::RegidragoVstar) == 0 && hand_count(Card::Crispin) == 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
  }

  bool issue_1811_inspected_arven_route_complete() const {
    if (!deck_seen_ || !issue_1811_public_arven_route_candidate()) return false;
    const int grass = deck_count_after_search_started(Card::Grass);
    const int fire = deck_count_after_search_started(Card::Fire);
    return deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
        deck_count_after_search_started(Card::ForestSealStone) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        grass > 0 && fire > 0 && grass + fire >= 3;
  }

  bool play_issue_1811_arven_route() {
    if (!issue_1811_public_arven_route_candidate()) return false;

    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = true;
    projected.prizes_revealed_ = prizes_revealed_;
    if (!projected.issue_1811_inspected_arven_route_complete()) return false;

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-1811 route");
    if (!move_deck_to_hand(Card::MysteriousTreasure) ||
        !move_deck_to_hand(Card::ForestSealStone)) {
      throw std::logic_error("projected issue-1811 Arven targets disappeared");
    }
    issue_1811_route_turn_ = state_.turn;
    shuffle(state_.deck);
    trace("PLAY SUPPORTER",
          "R-ARVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Searched Mysterious Treasure and Forest Seal Stone for the deterministic T2 Crispin route.");
    return true;
  }

  bool play_issue_1811_mysterious_treasure_route() {
    if (issue_1811_route_turn_ != state_.turn || item_locked() ||
        hand_count(Card::MysteriousTreasure) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 || !deck_seen_ ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0) {
      return false;
    }

    // Arven's K1 inspection proves that T2 Crispin replaces the held Fire with a
    // searched Grass and Fire pair. The original Fire is therefore dynamic-DCI
    // fuel for Treasure while Earthen Vessel preserves Dragapult ex for T2 JIT:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official discard-cost, search, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Dynamic DCI and strict-JIT specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1811
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::Fire, "Mysterious Treasure issue-1811 cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1811 Fire cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1811 route");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("projected issue-1811 Regidrago VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded route-replaced Fire Energy and searched Regidrago VSTAR.");
    return true;
  }

  bool use_issue_1811_fss_route() {
    const auto live_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    const bool live_fss = !state_.vstar_power_used &&
        ((state_.active && live_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live_holder));
    if (issue_1811_route_turn_ != state_.turn || !live_fss || !deck_seen_ ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        hand_count(Card::Crispin) > 0 || hand_count(Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Star Alchemy may search any card. Arven already used the T1 Supporter action,
    // so Crispin is banked for T2, when it supplies both required Energy types:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official VSTAR Power, Supporter, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1811
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    record_deck_search_knowledge("Star Alchemy issue-1811 route");
    if (!move_deck_to_hand(Card::Crispin)) {
      throw std::logic_error("projected issue-1811 Crispin target disappeared");
    }
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Banked Crispin for the deterministic T2 Grass-and-Fire attachment route.");
    return true;
  }

  bool hold_issue_1811_celestial_roar() {
    if (issue_1811_route_turn_ != state_.turn || !state_.active ||
        state_.active->card != Card::RegidragoV || state_.active->grass != 1 ||
        state_.active->fire != 0 || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Every T2 axis is public and deterministic. Celestial Roar cannot improve the
    // T2 deadline and may discard a required singleton or payload connector:
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Resource-preserving earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1811
    trace("HOLD ATTACK", "R-RV-01; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Held Celestial Roar because VSTAR, Crispin, Earthen Vessel, and the strict-JIT payload already guarantee T2 readiness.");
    return true;
  }

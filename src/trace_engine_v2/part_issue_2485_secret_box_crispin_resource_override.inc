  bool issue_2485_direct_crispin_preflight() const {
    const auto recipe_copies = [this](const Card card) {
      int copies = 0;
      for (const auto& [candidate, count] : recipe_) {
        if (candidate == card) copies += count;
      }
      return copies;
    };
    const bool registered_pineco_recipe =
        recipe_copies(Card::SecretBox) == 1 &&
        recipe_copies(Card::Pineco) == 2 &&
        recipe_copies(Card::ForretressEx) == 2;

    // This comparator is limited to the exact public resource shape in which the
    // existing full Pineco line is already legal, while Secret Box can instead use
    // its independent Item and Supporter searches for Mysterious Treasure and
    // Crispin. Crispin must still have two different Basic Energy types in deck;
    // one is attached by Crispin and the other is available for the unused manual
    // attachment, taking the prior-turn one-Grass Regidrago V to Apex Dragon's GGF.
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Crispin one-Energy ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Forretress ex / two-Prize self-KO comparator: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Official Item, Supporter, Energy, search, discard, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2485
    return registered_pineco_recipe && secret_box_combo_enabled() &&
        scenario_.dci == DciProfile::StrictJit && state_.turn == 2 &&
        !state_.turn_ended && supporter_allowed() && !state_.manual_energy_used &&
        !item_locked() && deck_seen_ && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 0 &&
        state_.active->double_dragon == 0 &&
        hand_count(Card::SecretBox) > 0 &&
        hand_count(Card::MysteriousTreasure) == 0 &&
        hand_count(Card::Crispin) == 0 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        in_play_count(Card::Pineco) == 0 &&
        in_play_count(Card::ForretressEx) == 0 &&
        deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        secret_box_full_combo_axes_visible();
  }

  bool execute_issue_2485_direct_crispin_route() {
    if (!issue_2485_direct_crispin_preflight()) return false;

    const auto costs = secret_box_cost_plan();
    if (!costs || !std::any_of(costs->begin(), costs->end(), is_payload)) {
      return false;
    }

    remove_one(state_.hand, Card::SecretBox);
    for (const Card cost : *costs) {
      if (!discard_from_hand(cost, "Secret Box issue-2485 direct-route cost",
                             "R-SECRET-BOX-01; P-DCI-01")) {
        throw std::logic_error("Issue 2485 Secret Box cost plan diverged from hand");
      }
    }
    state_.discard.push_back(Card::SecretBox);
    outcome_.used_secret_box = true;
    ++outcome_.secret_box_combo_attempted;
    record_deck_search_knowledge("Secret Box issue-2485 direct route");

    // Secret Box may search up to one card from each printed category. The direct
    // route deliberately takes only its Item and Supporter channels, preserving the
    // Pineco, Forretress ex, Forest of Vitality, and Tool resources because they do
    // not improve the T2 completion window.
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2485
    const bool took_treasure = move_deck_to_hand(Card::MysteriousTreasure);
    const bool took_crispin = move_deck_to_hand(Card::Crispin);
    shuffle(state_.deck);
    if (!took_treasure || !took_crispin) {
      throw std::logic_error("Issue 2485 projected Secret Box targets disappeared");
    }
    trace("PLAY ITEM", "R-SECRET-BOX-01; R-MT-01; R-CRISPIN-01; P-DCI-01",
          "Secret Box discarded three other cards and searched Mysterious "
          "Treasure plus Crispin for the resource-preserving direct T2 route.");

    if (!play_crispin()) return false;
    attach_manual();
    if (need_energy() || hand_count(Card::MysteriousTreasure) == 0 || item_locked()) {
      return false;
    }

    // After Crispin has consumed the turn's Supporter action and GGF is complete,
    // the remaining Treasure cost may use the ordinary DCI selector. The exact
    // seed-40 line also treats a held Gladion or Steven's Resolve as route-complete
    // Supporter fodder because neither can be played this turn and all setup axes
    // except the already-known VSTAR search are complete. This is a narrow dynamic
    // DCI state, not a generic permission to discard singleton Supporters.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // One Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2485
    std::optional<Card> treasure_cost = choose_discard(
        false, true, true, Card::MysteriousTreasure);
    if (!treasure_cost && state_.supporter_used) {
      for (const Card supporter : {Card::Gladion, Card::StevensResolve}) {
        if (hand_count(supporter) > 0) {
          treasure_cost = supporter;
          break;
        }
      }
    }
    if (!treasure_cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    if (!discard_from_hand(*treasure_cost,
                           "Mysterious Treasure issue-2485 direct-route cost",
                           "R-MT-01; P-DCI-01")) {
      throw std::logic_error("Issue 2485 Treasure cost disappeared");
    }
    state_.discard.push_back(Card::MysteriousTreasure);
    record_deck_search_knowledge("Mysterious Treasure issue-2485 direct route");
    if (!move_deck_to_hand(Card::RegidragoVstar)) return false;
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          "Mysterious Treasure discarded " + std::string(name(*treasure_cost)) +
          " and searched Regidrago VSTAR for the direct T2 route.");

    // Crispin supplies one legal effect attachment and one Basic Energy to hand;
    // the unused manual attachment supplies the third GGF unit. The Secret Box cost
    // already put a Dragon into discard this turn, satisfying strict-JIT without
    // Exploding Energy, and the searched VSTAR can evolve the prior-turn Regidrago V.
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, Item, Supporter, discard, search, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2485
    evolve_best_regi();
    return active_is_vstar() && !need_energy() && !need_payload();
  }

  bool play_issue_2485_direct_crispin_route() {
    if (!issue_2485_direct_crispin_preflight()) return false;

    // Prove the entire paid route on a copy first. This reuses the production DCI,
    // Crispin, manual-attachment, Treasure cost, evolution, and strict-JIT state, so
    // the live game is untouched unless every legal step reaches the same T2 ready
    // state while preserving Forretress ex.
    // No-future-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2485
    Engine projected = *this;
    projected.trace_ = nullptr;
    if (!projected.execute_issue_2485_direct_crispin_route()) return false;

    trace("POLICY", "P-AXIS-01; P-COMPRESS-01; R-SECRET-BOX-01",
          "The direct Secret Box, Crispin, and Mysterious Treasure route reaches "
          "T2 without spending the two-Prize Forretress ex line: " + state_line());
    if (!execute_issue_2485_direct_crispin_route()) {
      throw std::logic_error("Issue 2485 live route diverged from its K1 projection");
    }
    trace("POLICY", "P-AXIS-01; P-COMPRESS-01; R-SECRET-BOX-01; R-CRISPIN-01; R-MT-01",
          "Completed the resource-preserving direct T2 route; Pineco and "
          "Forretress ex remain unspent: " + state_line());
    return true;
  }

  void run_secret_box_turn_issue2485() {
    if (play_issue_2485_direct_crispin_route()) return;
    run_secret_box_turn_issue1816();
  }

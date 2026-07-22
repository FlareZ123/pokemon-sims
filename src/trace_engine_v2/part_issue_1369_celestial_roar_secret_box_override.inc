  bool guaranteed_next_turn_secret_box_route_after_hold() const {
    // This hold is restricted to the registered strict-JIT Pineco recipe after
    // legal K1 inspection. Read recipe_ directly because total_copies() deliberately
    // derives synthetic unit-fixture cards from zones when a card is absent from the
    // registered list. A shell fixture must never impersonate the Pineco recipe:
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Registered recipes: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L71-L150
    // Synthetic fixture copy accounting: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L78-L93
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1369
    // Validated fix: https://github.com/FlareZ123/pokemon-sims/pull/1371
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

    // Regidrago V already has the Energy required to attack, while Secret Box
    // and the held Fire/Grass schedule can complete the next-turn route:
    // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Core draw, Item, Supporter, evolution, Ability, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Secret Box route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1369
    if (!registered_pineco_recipe || !secret_box_combo_enabled() ||
        !strict_payload_timing() || !deck_seen_ ||
        state_.turn >= scenario_.max_turn || state_.turn_ended ||
        first_turn_going_first() || !state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->grass != 1 || state_.active->fire != 0 ||
        hand_count(Card::SecretBox) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::Grass) == 0 || state_.deck.empty()) {
      return false;
    }

    // Prove the route for every possible start-of-turn draw rather than reading the
    // randomized top card. Each projection moves one physically possible card from
    // deck to hand, advances public turn flags, and reuses the existing full-axis and
    // DCI cost planners. The hold is admitted only when every projection remains legal:
    // Start-of-turn draw: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-future-card oracle: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // K1 knowledge state: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Secret Box DCI planner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1118_secret_box.inc
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1369
    for (std::size_t draw_index = 0; draw_index < state_.deck.size();
         ++draw_index) {
      Engine projected = *this;
      projected.trace_ = nullptr;
      const Card drawn = projected.state_.deck[draw_index];
      projected.state_.deck.erase(projected.state_.deck.begin() +
                                  static_cast<std::ptrdiff_t>(draw_index));
      projected.state_.hand.push_back(drawn);
      ++projected.state_.turn;
      projected.state_.supporter_used = false;
      projected.state_.manual_energy_used = false;
      projected.state_.retreat_used = false;
      projected.state_.stadium_used = false;
      projected.state_.turn_ended = false;
      projected.state_.discarded_this_turn.clear();

      if (!projected.secret_box_full_combo_axes_visible() ||
          !projected.secret_box_cost_plan().has_value()) {
        return false;
      }
    }
    return true;
  }

  bool use_celestial_roar() {
    if (guaranteed_next_turn_secret_box_route_after_hold()) {
      // Celestial Roar cannot improve the earliest legal ready turn beyond T2 in
      // this graph. Its random discard can remove a complete required category,
      // while holding preserves the route across every possible next draw:
      // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1369
      trace("HOLD ATTACK",
            "R-RV-01; R-SECRET-BOX-01; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
            "Every possible next draw preserves a payable Secret Box, Dawn, Forest of Vitality, Forretress ex, Mysterious Treasure, VSTAR, GGF, and strict-JIT payload route.");
      return false;
    }
    return use_celestial_roar_issue1369_original();
  }

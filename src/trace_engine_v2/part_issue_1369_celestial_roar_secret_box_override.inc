  // Restore the public Pineco turn-policy name after the issue-1118 body, then
  // install the exact issue-1439 route comparator before later overrides:
  // https://github.com/FlareZ123/pokemon-sims/issues/1439
  #undef run_secret_box_turn
  #include "part_issue_1439_treasure_tapu_crispin_override.inc"

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

  bool issue_1719_earthen_vessel_next_window_route() const {
    if (!deck_seen_ || !strict_payload_timing() || state_.turn_ended ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        !state_.manual_energy_used || state_.turn >= scenario_.max_turn) {
      return false;
    }

    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    const bool item_available_next_turn =
        scenario_.locks != LockMode::FullItem &&
        scenario_.locks != LockMode::FullCombined &&
        !(scenario_.locks == LockMode::TurnTwoItem && state_.turn + 1 >= 2);
    const bool required_energy_in_known_deck =
        (missing_grass == 1 &&
         deck_count_after_search_started(Card::Grass) > 0) ||
        (missing_fire == 1 &&
         deck_count_after_search_started(Card::Fire) > 0);
    const bool held_payload_cost =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool live_vstar_axis =
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) > 0;

    // Earthen Vessel already covers the one missing Basic Energy and the strict-JIT
    // payload outlet on the next legal evolution window. Celestial Roar cannot change
    // the marginal identity of the next unknown draw, while its random top-three
    // processing can permanently remove Regidrago VSTAR and search outs before that
    // live axis is reached. Hold only from K1 when the Vessel cost, searched Energy,
    // future manual attachment, Item access, and VSTAR axis are all observable:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, attachment, evolution, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Observable-state policy, strict-JIT, dynamic DCI, and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1719
    return missing_grass + missing_fire == 1 &&
        hand_count(Card::EarthenVessel) > 0 && item_available_next_turn &&
        required_energy_in_known_deck && held_payload_cost && live_vstar_axis;
  }

  bool use_celestial_roar() {
    if (issue_1719_earthen_vessel_next_window_route()) {
      // The held Vessel searches the sole missing Basic Energy and discards the held
      // Dragon payload during the next strict-JIT ready turn. Random top-three
      // processing cannot improve that earliest legal route and may remove its live
      // VSTAR or search axis:
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official attack, Item, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1719
      trace("HOLD ATTACK",
            "R-RV-01; R-EV-01; R-GAME-ENERGY; P-KNOWLEDGE-01; P-JIT-01; P-COMPRESS-01",
            "A payable Earthen Vessel, held Dragon payload, and live VSTAR axis already cover the next legal ready window.");
      return false;
    }
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

  // Compose the scheduled Item-lock route after the ordinary Pineco policy, then
  // rewrite only the later registered-deck dispatch call to the narrow wrapper:
  // https://github.com/FlareZ123/pokemon-sims/issues/1704
  #include "part_issue_1704_item_lock_burnet_override.inc"
  #define run_secret_box_turn run_secret_box_turn_issue1704


  // Compose the confirmed K1 pre-Steven route after the scheduled Item-lock wrapper,
  // then redirect the later general-policy probe through its narrow dispatcher:
  // https://github.com/FlareZ123/pokemon-sims/issues/1705
  #include "part_issue_1705_treasure_latias_vessel_override.inc"
  #define late_steven_has_known_t3_compression_route \
    late_steven_has_known_t3_compression_route_issue1705


  // Compose the confirmed K1 Heavy Ball route after the Treasure-Latias dispatcher,
  // preserving the prior fix as the fallback for every non-matching state:
  // https://github.com/FlareZ123/pokemon-sims/issues/1702
  #undef late_steven_has_known_t3_compression_route
  #include "part_issue_1702_heavy_ball_latias_override.inc"
  #define late_steven_has_known_t3_compression_route \
    late_steven_has_known_t3_compression_route_issue1702

  // Compose the confirmed K0 Treasure-Turo-Crispin route after earlier Pineco
  // wrappers, then direct the registered-deck dispatcher through this narrow layer:
  // https://github.com/FlareZ123/pokemon-sims/issues/1711
  #include "part_issue_1711_k0_treasure_turo_crispin_override.inc"
  #undef run_secret_box_turn
  #define run_secret_box_turn run_secret_box_turn_issue1711

  // Compare the one-cost Quick Ball payload route before the final Pineco dispatcher
  // can spend Secret Box's three-card ACE SPEC cost:
  // https://github.com/FlareZ123/pokemon-sims/issues/1744
  #include "part_issue_1744_quick_ball_forretress_override.inc"
  #undef run_secret_box_turn
  #define run_secret_box_turn run_secret_box_turn_issue1744

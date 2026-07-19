  bool wonder_tag_duplicate_held_arven_has_no_marginal_route() const {
    if (!deck_seen_ || !supporter_allowed() ||
        hand_count(Card::TapuLeleGX) == 0 || hand_count(Card::Arven) == 0 ||
        deck_count_after_search_started(Card::Arven) == 0 || !need_energy() ||
        state_.manual_energy_used || item_locked()) {
      return false;
    }

    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_in_deck =
        (grass_needed() == 1 &&
         deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 &&
         deck_count_after_search_started(Card::Fire) > 0);
    const bool duplicate_arven_would_repeat_vessel_route =
        exactly_one_energy_missing && needed_energy_in_deck &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        is_legal_cost_available(can_play_payload_this_turn(), true);
    if (!duplicate_arven_would_repeat_vessel_route) return false;

    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    const int arven_discard_before =
        count_of(projected.state_.discard, Card::Arven);
    if (!remove_one(projected.state_.hand, Card::TapuLeleGX)) return false;
    projected.run_turn();

    const bool played_held_arven = projected.state_.supporter_used &&
        count_of(projected.state_.discard, Card::Arven) > arven_discard_before;
    const bool completed_without_second_tapu = projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();

    // Wonder Tag searches one Supporter, while the held Arven already executes the
    // same Earthen Vessel route. Projecting the exact K1 state without the second
    // Tapu proves that the held copy completes Energy, evolution, payload, and
    // Active-position axes before suppressing the one-shot Bench connector:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // One Supporter, Items, Tools, and manual attachment: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1045
    return played_held_arven && completed_without_second_tapu;
  }

  bool bench_tapu_if_useful() {
    if (wonder_tag_duplicate_held_arven_has_no_marginal_route()) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1045-duplicate-held-arven", state_.turn,
            "HOLD TAPU LELE-GX",
            "R-TAPU-01; R-GAME-SUPPORTER; P-CONNECTOR-01; P-KNOWLEDGE-01",
            "Retained the second Tapu because held Arven already completes the identical observable Item and Tool route.");
      }
      return false;
    }

    // The active empty-deck guard must consult the complete-route-aware connector
    // predicate before delegating to the existing physical bench and search checks:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/989
    if (!needs_tapu_connector()) return false;
    return bench_tapu_if_useful_issue989_empty_deck_original();
  }

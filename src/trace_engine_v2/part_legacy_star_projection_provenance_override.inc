  std::optional<Card> legacy_star_projection_removed_marker_;

  bool is_legacy_star_discard_projection(const std::vector<Card>& zone) const {
    return &zone == &state_.discard;
  }

  bool remove_one_legacy_star_projection(std::vector<Card>& zone, const Card card) {
    const bool is_discard_recovery_projection = is_legacy_star_discard_projection(zone);
    auto& discarded_this_turn = state_.discarded_this_turn;
    const int discard_copies = is_discard_recovery_projection ? count_of(state_.discard, card) : 0;
    const int current_turn_copies = is_discard_recovery_projection
        ? count_of(discarded_this_turn, card)
        : 0;
    if (!remove_one(zone, card)) return false;

    // Pokémon Communication preflight projects a Legacy Star recovery before it
    // scores the exchange Pokémon. Keep the copied discard and same-turn provenance
    // synchronized with the live recovery transition, including duplicate selection:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/936
    if (is_discard_recovery_projection && current_turn_copies > 0 &&
        discard_copies == current_turn_copies) {
      const auto turn_it = std::find(
          discarded_this_turn.begin(), discarded_this_turn.end(), card);
      discarded_this_turn.erase(turn_it);
      legacy_star_projection_removed_marker_ = card;
    }
    return true;
  }

  void restore_legacy_star_projection_provenance() {
    // Candidate scoring is speculative. Restore the projected provenance change
    // immediately after the plan observes it so the live Legacy Star state remains
    // unchanged until an actual recovery is selected:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/issues/936
    if (!legacy_star_projection_removed_marker_) return;
    state_.discarded_this_turn.push_back(*legacy_star_projection_removed_marker_);
    legacy_star_projection_removed_marker_.reset();
  }

  std::optional<PokemonCommunicationPlan>
  pokemon_communication_plan_legacy_star_projection(
      const bool permit_payload, const bool communication_is_held,
      const bool supporter_will_be_used) {
    const auto plan = pokemon_communication_plan(
        permit_payload, communication_is_held, supporter_will_be_used);
    restore_legacy_star_projection_provenance();
    return plan;
  }

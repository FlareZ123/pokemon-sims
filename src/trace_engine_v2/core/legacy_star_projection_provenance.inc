  struct LegacyStarProjectionProvenance {
    std::optional<Card> removed_card;

    bool should_remove(const std::vector<Card>& discard,
                       const std::vector<Card>& discarded_this_turn,
                       const Card card) const {
      const int current_turn_copies = count_of(discarded_this_turn, card);
      const int discard_copies = count_of(discard, card);
      return current_turn_copies > 0 && discard_copies == current_turn_copies;
    }

    void mark_recovery(std::vector<Card>& discarded_this_turn, const Card card) {
      const auto turn_it = std::find(
          discarded_this_turn.begin(), discarded_this_turn.end(), card);
      discarded_this_turn.erase(turn_it);
      removed_card = card;
    }

    void restore(std::vector<Card>& discarded_this_turn) {
      if (!removed_card) return;
      discarded_this_turn.push_back(*removed_card);
      removed_card.reset();
    }
  };

  LegacyStarProjectionProvenance legacy_star_projection_provenance_;

  bool should_remove_legacy_star_projection_provenance(
      const std::vector<Card>& zone, const Card card) const {
    if (&zone != &state_.discard) return false;
    return legacy_star_projection_provenance_.should_remove(
        state_.discard, state_.discarded_this_turn, card);
  }

  bool remove_one_legacy_star_projection(std::vector<Card>& zone, const Card card) {
    const bool remove_current_turn_provenance =
        should_remove_legacy_star_projection_provenance(zone, card);
    if (!remove_one(zone, card)) return false;

    // Pokémon Communication preflight projects a Legacy Star recovery before it
    // scores the exchange Pokémon. Keep the copied discard and same-turn provenance
    // synchronized with the live recovery transition, including duplicate selection:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/936
    if (remove_current_turn_provenance) {
      legacy_star_projection_provenance_.mark_recovery(
          state_.discarded_this_turn, card);
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
    legacy_star_projection_provenance_.restore(state_.discarded_this_turn);
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

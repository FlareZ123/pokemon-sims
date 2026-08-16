  int total_energy_units(const Pokemon& pokemon) const {
    // Each physical DDE supplies exactly two Energy of every type while
    // legally attached to Dragon Pokémon:
    // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // https://github.com/FlareZ123/pokemon-sims/issues/2238
    return pokemon.grass + pokemon.fire + 2 * pokemon.double_dragon;
  }

  bool pays_apex_energy_cost(const Pokemon& pokemon) const {
    // Without DDE, retain exact GGF typing. With at least one legally
    // attached DDE, its all-type provision can satisfy whichever typed
    // components remain as long as three total Energy units exist:
    // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2238
    if (pokemon.double_dragon <= 0) {
      return pokemon.grass >= 2 && pokemon.fire >= 1;
    }
    return is_dragon(pokemon.card) && total_energy_units(pokemon) >= 3;
  }

  int apex_energy_progress(const Pokemon& pokemon) const {
    if (pays_apex_energy_cost(pokemon)) return 3;
    if (pokemon.double_dragon > 0 && is_dragon(pokemon.card)) {
      return std::min(2, total_energy_units(pokemon));
    }
    return std::min(2, pokemon.grass) + std::min(1, pokemon.fire);
  }

  bool attach_energy_card(Pokemon& pokemon, const Card energy) const {
    if (energy == Card::Grass) { ++pokemon.grass; return true; }
    if (energy == Card::Fire) { ++pokemon.fire; return true; }
    if (energy == Card::DoubleDragonEnergy && is_dragon(pokemon.card)) {
      ++pokemon.double_dragon;
      return true;
    }
    return false;
  }

  bool basic_attachment_completes_apex(const Pokemon& target, const Card energy) const {
    // Grass and Fire are the Basic Energy types in the modeled Regidrago shell.
    // Project the physical attachment through the same DDE-aware Apex payment helper
    // instead of treating grass_needed()/fire_needed() as additive card deficits:
    // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Umbrella migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
    if (energy != Card::Grass && energy != Card::Fire) return false;
    Pokemon projected = target;
    return attach_energy_card(projected, energy) && pays_apex_energy_cost(projected);
  }

  template <typename Available>
  std::optional<Card> completing_basic_energy_for(
      const Pokemon& target, Available available) const {
    if (pays_apex_energy_cost(target)) return std::nullopt;
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (available(basic) && basic_attachment_completes_apex(target, basic)) {
        return basic;
      }
    }
    return std::nullopt;
  }

  std::optional<Card> completing_basic_energy_from_known_deck(
      const Pokemon& target) const {
    return completing_basic_energy_for(target, [this](const Card card) {
      return deck_count_after_search_started(card) > 0;
    });
  }

  std::optional<Card> completing_basic_energy_from_possible_deck(
      const Pokemon& target) const {
    return completing_basic_energy_for(
        target, [this](const Card card) { return might_be_unseen(card); });
  }

  int minimum_basic_attachments_to_apex(
      const Pokemon& target, const int grass_available,
      const int fire_available) const {
    if (pays_apex_energy_cost(target)) return 0;
    // grass_needed()/fire_needed() are typed *options* when DDE is attached, not
    // additive physical-card deficits. Brute-force the at-most-three Basic Energy
    // attachments relevant to Apex Dragon and return physical card distance:
    // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Umbrella migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
    for (int count = 1; count <= 3; ++count) {
      for (int grass = 0; grass <= count; ++grass) {
        const int fire = count - grass;
        if (grass > grass_available || fire > fire_available) continue;
        Pokemon projected = target;
        for (int i = 0; i < grass; ++i) attach_energy_card(projected, Card::Grass);
        for (int i = 0; i < fire; ++i) attach_energy_card(projected, Card::Fire);
        if (pays_apex_energy_cost(projected)) return count;
      }
    }
    return 4;
  }

  template <typename Available>
  std::optional<Card> preferred_manual_energy_for(
      const Pokemon& target, Available available) const {
    if (!is_dragon(target.card) || pays_apex_energy_cost(target)) {
      return std::nullopt;
    }
    const auto projected_progress = [this, &target](const Card card) {
      Pokemon projected = target;
      if (!attach_energy_card(projected, card)) return -1;
      return apex_energy_progress(projected);
    };
    // If a Basic Energy and DDE both finish this turn, spend the Basic
    // and preserve DDE's higher future AMR/compression value:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/2238
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (!available(basic)) continue;
      Pokemon projected = target;
      if (attach_energy_card(projected, basic) &&
          pays_apex_energy_cost(projected)) return basic;
    }
    Card best = Card::Grass;
    int best_progress = apex_energy_progress(target);
    bool found = false;
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (!available(basic)) continue;
      const int progress = projected_progress(basic);
      if (!found || progress > best_progress) {
        best = basic;
        best_progress = progress;
        found = true;
      }
    }
    if (available(Card::DoubleDragonEnergy)) {
      const int dde_progress = projected_progress(Card::DoubleDragonEnergy);
      // DDE is selected only for a strictly stronger Energy-axis result.
      // Equal progress remains on a Basic to preserve the two-unit Special:
      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // https://github.com/FlareZ123/pokemon-sims/issues/2238
      if (!found || dde_progress > best_progress) {
        best = Card::DoubleDragonEnergy;
        best_progress = dde_progress;
        found = true;
      }
    }
    return found && best_progress > apex_energy_progress(target)
        ? std::optional<Card>{best}
        : std::nullopt;
  }

  std::optional<Card> preferred_manual_energy_from_hand(
      const Pokemon& target) const {
    return preferred_manual_energy_for(
        target, [this](const Card card) { return hand_count(card) > 0; });
  }

  std::optional<Card> preferred_manual_energy_from_known_deck(
      const Pokemon& target) const {
    return preferred_manual_energy_for(target, [this](const Card card) {
      return deck_count_after_search_started(card) > 0;
    });
  }

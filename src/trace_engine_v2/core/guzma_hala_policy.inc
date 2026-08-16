#pragma once

  struct GuzmaHalaSearchPlan {
    Engine* engine;
    bool want_stadium_unlock;
    bool want_fss;
    bool want_dde;
  };

  bool guzma_hala_has_regidrago_energy_holder() const {
    // Keep whole-board presence checks on the canonical Active-first traversal.
    // Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    return board_has_pokemon_matching([](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV ||
             pokemon.card == Card::RegidragoVstar;
    });
  }

  bool guzma_hala_has_open_fss_holder() const {
    // Reuse the canonical Pokemon V Tool-slot query instead of repeating board traversal.
    // Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    return board_has_open_pokemon_v_tool_slot();
  }

  bool guzma_hala_projected_dde_vstar_route() const {
    if (state_.manual_energy_used || hand_count(Card::RegidragoVstar) == 0) {
      return false;
    }

    const auto dde_finishes = [this](const Pokemon& pokemon) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        return false;
      }
      Pokemon projected = pokemon;
      projected.card = Card::RegidragoVstar;
      if (pays_apex_energy_cost(projected)) return false;
      return attach_energy_card(projected, Card::DoubleDragonEnergy) &&
             pays_apex_energy_cost(projected);
    };

    // Active Regidrago V needs no promotion edge. A Benched prior-turn Regidrago V
    // is equally live when Latias ex gives the current Basic Active free Retreat;
    // after G&H searches DDE the engine may attach, evolve, retreat, and attack in
    // the same turn. This is a board-state projection rather than a seat/seed route.
    // Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Retreat/evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (state_.active && dde_finishes(*state_.active)) return true;
    if (state_.retreat_used || !state_.active || !can_free_retreat_with_latias()) {
      return false;
    }
    return std::any_of(state_.bench.begin(), state_.bench.end(), dde_finishes);
  }

  bool guzma_hala_dde_axis_available() const {
    // Double Dragon Energy is Special Energy and may attach only to Dragon Pokemon.
    // The ordinary energy-axis predicate covers an already-evolved Regidrago VSTAR.
    // The projected route also covers a prior-turn Regidrago V in Active or Bench
    // when same-turn evolution and legal Latias promotion make DDE complete Apex.
    // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/issues/3386
    if (state_.manual_energy_used || !guzma_hala_has_regidrago_energy_holder() ||
        hand_count(Card::DoubleDragonEnergy) > 0 ||
        !might_be_unseen(Card::DoubleDragonEnergy)) {
      return false;
    }
    if (need_energy()) return true;
    return guzma_hala_projected_dde_vstar_route();
  }

  bool guzma_hala_fss_axis_available() const {
    // Forest Seal Stone supplies a one-per-game any-card search from a Pokemon V.
    // Do not spend the two-card Guzma & Hala bonus for a Tool whose VSTAR Power is
    // already spent or whose holder is unavailable.
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    return setup_axis_missing() && !state_.vstar_power_used &&
           guzma_hala_has_open_fss_holder() &&
           hand_count(Card::ForestSealStone) == 0 &&
           might_be_unseen(Card::ForestSealStone);
  }

  bool guzma_hala_stadium_unlock_available() const {
    // Chaotic Swell can replace the modeled Path to the Peak and restore Rule Box
    // Pokemon Abilities. Stadium use is independent of Item lock.
    // https://api.pokemontcg.io/v2/cards/sm12-187
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    return rule_box_abilities_locked() && !state_.stadium_used &&
           state_.stadium != Stadium::ChaoticSwell &&
           hand_count(Card::ChaoticSwell) == 0 &&
           might_be_unseen(Card::ChaoticSwell);
  }

  bool guzma_hala_payload_cost_can_finish_this_turn() const {
    if (can_play_payload_this_turn()) return true;

    // Before evolution, the generic payload predicate does not yet apply because
    // Regidrago V has no Apex Dragon requirement. Project only the complete route:
    // a prior-turn Regidrago V evolves from the held VSTAR, searched DDE completes
    // GGF, and (when Benched) Latias ex supplies the legal free promotion. No Dragon
    // payload may already have been established this turn. This preserves strict
    // same-turn JIT semantics without keying policy to a particular hand or seed.
    // Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dynamic JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3386
    if (!strict_payload_timing() || state_.manual_energy_used ||
        std::any_of(state_.discarded_this_turn.begin(),
                    state_.discarded_this_turn.end(), is_payload) ||
        !guzma_hala_projected_dde_vstar_route()) {
      return false;
    }
    return true;
  }

  std::optional<std::array<Card, 2>> guzma_hala_bonus_costs(
      const bool permit_payload) {
    if (hand_count(Card::GuzmaHala) == 0) return std::nullopt;

    const std::vector<Card> original_hand = state_.hand;
    if (!remove_one(state_.hand, Card::GuzmaHala)) return std::nullopt;

    // Reuse the simulator's dynamic DCI/UDP selector rather than assigning fixed
    // discard values to Guzma & Hala. The first selected payload may satisfy JIT;
    // the normal second-cost pass preserves all remaining payload identities.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool flex_fodder = scenario_.dci == DciProfile::MatchupFlexJit;
    const auto first = choose_discard(permit_payload, flex_fodder);
    if (!first || !remove_one(state_.hand, *first)) {
      state_.hand = original_hand;
      return std::nullopt;
    }
    std::optional<Card> second = choose_discard(
        permit_payload && !is_payload(*first), flex_fodder);

    // Once the first cost is the proven current-turn payload, future-only value is
    // subordinate to completing readiness now. Ask the existing broader selector
    // first, then exploit the one-Supporter-per-turn rule: after Guzma & Hala has
    // been chosen, every other held Supporter is unusable this turn and therefore a
    // sound second cost for a route already proven to become ready this turn.
    // This is card-type policy rather than a named-card exception and does not use
    // hidden deck or Prize identities.
    // Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3386
    if (!second && permit_payload && is_payload(*first)) {
      second = choose_discard(false, true);
    }
    if (!second && permit_payload && is_payload(*first)) {
      const auto supporter = std::find_if(
          state_.hand.begin(), state_.hand.end(), [](const Card card) {
            return is_supporter(card);
          });
      if (supporter != state_.hand.end()) second = *supporter;
    }

    // If the first current-turn JIT cost is a Dragon payload and no ordinary or
    // Supporter cost survives, an additional physical copy of that same payload is
    // the lowest-loss fallback. Spending the duplicate preserves every distinct
    // Apex Dragon attack identity while allowing deterministic DDE + Tool compression.
    // Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dynamic DCI and discrete-resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (!second && permit_payload && is_payload(*first) &&
        hand_count(*first) > 0) {
      second = *first;
    }

    state_.hand = original_hand;
    if (!second) return std::nullopt;
    return std::array<Card, 2>{*first, *second};
  }

  static cards::GuzmaHala::SearchTargets select_guzma_hala_targets(
      void* opaque) {
    auto& plan = *static_cast<GuzmaHalaSearchPlan*>(opaque);
    Engine& engine = *plan.engine;
    cards::GuzmaHala::SearchTargets targets;

    // This selector runs only after GuzmaHala::resolve begins the legal deck search,
    // so every choice below is K1 and checks the actual remaining deck rather than
    // inferred Prize locations.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    const auto in_deck = [&engine](const Card card) {
      return std::find(engine.state_.deck.begin(), engine.state_.deck.end(), card) !=
             engine.state_.deck.end();
    };

    // The printed Stadium search is part of the base effect and costs no additional
    // hand resource once Guzma & Hala has been selected. After the legal inspection
    // establishes K1, take the best modeled Stadium that actually remains in deck
    // instead of deliberately failing a known productive search. Chaotic Swell is
    // preferred because it is safe into the shell's own Rule Box Pokemon; Forest of
    // Vitality is the next generally useful Stadium, and Path to the Peak remains a
    // legal last-resort hand resource when it is the only searchable Stadium.
    // Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    if (in_deck(Card::ChaoticSwell)) {
      targets.stadium = Card::ChaoticSwell;
    } else if (in_deck(Card::ForestOfVitality)) {
      targets.stadium = Card::ForestOfVitality;
    } else if (in_deck(Card::PathToPeak)) {
      targets.stadium = Card::PathToPeak;
    }

    if (plan.want_fss && in_deck(Card::ForestSealStone)) {
      targets.tool = Card::ForestSealStone;
    }
    if (plan.want_dde && in_deck(Card::DoubleDragonEnergy)) {
      targets.special_energy = Card::DoubleDragonEnergy;
    }
    return targets;
  }

  rules::CardContext make_guzma_hala_card_context() {
    return rules::CardContext{
        this,
        [](const void* opaque, const Card card) {
          return static_cast<const Engine*>(opaque)->hand_count(card);
        },
        [](void* opaque, const Card card) {
          auto* engine = static_cast<Engine*>(opaque);
          const auto it = std::find(engine->state_.hand.begin(),
                                    engine->state_.hand.end(), card);
          if (it == engine->state_.hand.end()) return false;
          engine->state_.discard.push_back(*it);
          engine->state_.hand.erase(it);
          return true;
        },
        [](void* opaque, const Card card, const std::string_view reason,
           const std::string_view rules_reference) {
          return static_cast<Engine*>(opaque)->discard_from_hand(
              card, reason, rules_reference);
        },
        [](void* opaque, const Card card) {
          return static_cast<Engine*>(opaque)->move_deck_to_hand(card);
        },
        [](void* opaque) {
          auto* engine = static_cast<Engine*>(opaque);
          engine->shuffle(engine->state_.deck);
        },
        [](const void*, const Card card) { return is_basic(card); },
        [](void* opaque, const std::string_view reason) {
          static_cast<Engine*>(opaque)->record_deck_search_knowledge(reason);
        },
        [](const void*, const Card card) { return is_stadium(card); },
        [](const void*, const Card card) { return is_tool(card); },
        [](const void*, const Card card) { return is_special_energy(card); }};
  }

  bool play_guzma_hala() {
    if (!supporter_allowed() || hand_count(Card::GuzmaHala) == 0) return false;

    const bool want_unlock = guzma_hala_stadium_unlock_available();
    const bool want_fss = guzma_hala_fss_axis_available();
    const bool want_dde = guzma_hala_dde_axis_available();
    const bool permit_payload = guzma_hala_payload_cost_can_finish_this_turn();
    const auto costs = guzma_hala_bonus_costs(permit_payload);
    const bool bonus_available = costs.has_value();

    // The optional cost is worthwhile only for deterministic multi-axis compression.
    // A lone DDE search must not displace Crispin, and a lone FSS search must not
    // displace Arven. Combining DDE + FSS, or either bonus axis with the Stadium
    // unlock, advances multiple independent setup resources with one Supporter.
    // Guzma & Hala: https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earliest/compression policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const int bonus_axes = static_cast<int>(want_fss) + static_cast<int>(want_dde);
    const bool use_bonus = bonus_available &&
        (bonus_axes >= 2 || (want_unlock && bonus_axes >= 1));
    if (!use_bonus && !want_unlock) return false;

    GuzmaHalaSearchPlan plan{.engine = this,
                             .want_stadium_unlock = want_unlock,
                             .want_fss = use_bonus && want_fss,
                             .want_dde = use_bonus && want_dde};
    cards::GuzmaHala::Action action;
    action.use_bonus = use_bonus;
    if (use_bonus) action.discards = *costs;
    action.search_context = &plan;
    action.choose_search_targets = &Engine::select_guzma_hala_targets;
    action.cost_reason = "Guzma & Hala optional two-card cost";
    action.rules_reference =
        "R-GUZMA-HALA-01; P-DCI-01; P-COMPRESS-01";
    action.search_reason = "Guzma & Hala";

    auto context = make_guzma_hala_card_context();
    const cards::GuzmaHala::Resolution resolution =
        cards::GuzmaHala::resolve(context, action);
    if (!resolution.played) return false;
    state_.supporter_used = true;

    std::vector<Card> found;
    if (resolution.found_stadium && resolution.search_targets.stadium) {
      found.push_back(*resolution.search_targets.stadium);
    }
    if (resolution.found_tool && resolution.search_targets.tool) {
      found.push_back(*resolution.search_targets.tool);
    }
    if (resolution.found_special_energy &&
        resolution.search_targets.special_energy) {
      found.push_back(*resolution.search_targets.special_energy);
    }

    std::string detail = "Searched: " + join_cards(found) + ".";
    if (use_bonus) {
      detail += " Discarded " + std::string(name((*costs)[0])) + " and " +
                std::string(name((*costs)[1])) + " for the bonus search.";
    }
    trace("PLAY SUPPORTER",
          "R-GUZMA-HALA-01; R-GAME-SUPPORTER; P-DCI-01; P-COMPRESS-01",
          detail);
    return true;
  }
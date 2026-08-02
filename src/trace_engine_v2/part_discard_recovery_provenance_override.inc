  // Crobat V should resolve after deterministic hand compression when the
  // current route can keep Dark Asset live. The flag is scoped by the final turn
  // policy and only suppresses the generic Basic loop's Crobat branch:
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Action-order policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
  bool issue_1751_defer_crobat_{false};

  bool recover_discard_to_hand(const Card card) {
    const int discard_copies = count_of(state_.discard, card);
    if (discard_copies == 0) return false;
    const int current_turn_copies = count_of(state_.discarded_this_turn, card);

    const auto discard_it = std::find(state_.discard.begin(), state_.discard.end(), card);
    state_.hand.push_back(*discard_it);
    state_.discard.erase(discard_it);

    // When an older copy exists, recovery can select it and preserve the current-turn
    // copy in discard. Remove provenance only when every matching discard copy entered
    // discard this turn:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/934
    if (current_turn_copies > 0 && discard_copies == current_turn_copies) {
      const auto turn_it = std::find(
          state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), card);
      state_.discarded_this_turn.erase(turn_it);
    }
    return true;
  }


  std::optional<Card> quick_ball_dialga_vessel_route_cost() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2090

    // Once Active Regidrago VSTAR already has GGF and the only missing axis is a
    // current-turn Dragon payload, Quick Ball may use a route-conditioned cost,
    // search Basic Dialga-GX, and let a separately held Earthen Vessel discard it.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
    // Opponent-action boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
    // Official Item, discard, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1237
    const bool complete_public_route =
        scenario_.dci == DciProfile::StrictJit && prizes_known() && !item_locked() &&
        active_is_vstar() && !need_regi() && !need_vstar() && !need_energy() &&
        !need_active_vstar() && need_payload() &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::EarthenVessel) > 0 &&
        deck_count_after_search_started(Card::DialgaGX) > 0 &&
        (deck_count_after_search_started(Card::Grass) > 0 ||
         deck_count_after_search_started(Card::Fire) > 0);
    if (!complete_public_route) return std::nullopt;

    if (hand_count(Card::Channeler) > 0) return Card::Channeler;
    if (hand_count(Card::Grass) > grass_needed()) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed()) return Card::Fire;
    return std::nullopt;
  }

  std::optional<Card> choose_discard_issue_1237(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard(permit_payload, flex_fodder,
                                         allow_heavy_ball, excluded_from_cost,
                                         allow_redundant_payload);
    if (ordinary || excluded_from_cost != Card::QuickBall) return ordinary;
    return quick_ball_dialga_vessel_route_cost();
  }

  bool has_payable_payload_outlet_after_costed_search_issue_1237(
      const Card excluded) {
    if (has_payable_payload_outlet_after_costed_search(excluded)) return true;
    return excluded == Card::QuickBall &&
           quick_ball_dialga_vessel_route_cost().has_value();
  }

#undef bench_crobat_if_useful

  bool fss_crobat_compression_available() {
    // Forest Seal Stone can be attached to a Pokémon V before Crobat V is played,
    // reducing the post-Bench hand by one while preserving Star Alchemy. Dark Asset
    // then draws one additional unknown card without reading future deck order:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Regidrago V and VSTAR are Pokémon V: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Tool, Bench, and Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Pokémon Tool errata: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    // Observable-state and connector priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1726
    if (hand_count(Card::ForestSealStone) == 0 ||
        hand_count(Card::CrobatV) == 0 || state_.vstar_power_used ||
        state_.dark_asset_used || bench_space() <= 0 || state_.deck.empty() ||
        state_.hand.size() > 7U ||
        !ability_available_for_pokemon(Card::CrobatV) ||
        !(need_regi() || need_vstar() || need_energy() || need_payload() ||
          need_active_vstar()) ||
        (hand_count(Card::Oricorio) > 0 && needs_oricorio_connector()) ||
        (hand_count(Card::TapuLeleGX) > 0 && needs_tapu_connector())) {
      return false;
    }

    // Preserve a live Powerglass attachment that immediately advances the Active
    // Regidrago's GGF axis. Both Tools compete for the same one-Tool slot:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const bool live_powerglass = hand_count(Card::Powerglass) > 0 && state_.active &&
        (state_.active->card == Card::RegidragoV ||
         state_.active->card == Card::RegidragoVstar) &&
        state_.active->tool == Tool::None &&
        ((state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0) ||
         (state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0));
    if (live_powerglass) return false;

    const auto open_regidrago = [](const Pokemon& pokemon) {
      return (pokemon.card == Card::RegidragoV ||
              pokemon.card == Card::RegidragoVstar) &&
          pokemon.tool == Tool::None;
    };
    return (state_.active && open_regidrago(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), open_regidrago);
  }

  bool attach_fss_for_crobat_compression() {
    if (!fss_crobat_compression_available()) return false;
    Pokemon* target = nullptr;
    for (Pokemon& pokemon : state_.bench) {
      if ((pokemon.card == Card::RegidragoV ||
           pokemon.card == Card::RegidragoVstar) &&
          pokemon.tool == Tool::None) {
        target = &pokemon;
        break;
      }
    }
    // Forest Seal Stone grants Star Alchemy from either position. Prefer an open
    // Benched Pokémon V so the Active Regidrago keeps its one Tool slot available
    // for Powerglass; retain the Active as the legal fallback when no Bench holder
    // exists:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Official Pokémon Tool procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1732
    if (target == nullptr && state_.active &&
        (state_.active->card == Card::RegidragoV ||
         state_.active->card == Card::RegidragoVstar) &&
        state_.active->tool == Tool::None) {
      target = &*state_.active;
    }
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::ForestSealStone);
    target->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; P-COMPRESS-01",
          "Attached Forest Seal Stone to Regidrago V before Crobat V to increase Dark Asset's draw.");
    return true;
  }

  bool bench_crobat_if_useful() {
    // Generic Basic placement remains available for deterministic connectors while
    // the issue-1751 turn policy defers only Crobat V until compression is complete:
    // https://github.com/FlareZ123/pokemon-sims/issues/1751
    if (issue_1751_defer_crobat_ || hand_count(Card::CrobatV) == 0 ||
        bench_space() <= 0) return false;
    attach_fss_for_crobat_compression();
    if (!needs_crobat_connector()) return false;
    return bench_from_hand(Card::CrobatV, true);
  }

// Limit the route-conditioned wrappers to Mysterious Treasure and Quick Ball.
// The macros are removed at the start of part_009b2.inc before Ultra Ball logic.
// https://github.com/FlareZ123/pokemon-sims/issues/1237
#define choose_discard choose_discard_issue_1237
#define has_payable_payload_outlet_after_costed_search \
  has_payable_payload_outlet_after_costed_search_issue_1237

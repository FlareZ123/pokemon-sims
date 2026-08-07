  bool issue_2271_surplus_regidrago_v_treasure_route_available(
      const std::optional<Card> excluded_from_cost) const {
    // K1 proves the complete current-turn Treasure -> Tapu -> Arven -> Vessel
    // continuation. Once an Active Regidrago VSTAR already owns the Basic,
    // evolution, and Active-position axes, the extra held Regidrago V has no setup
    // role in this route. Mysterious Treasure may spend it, then Tapu Lele-GX finds
    // Arven, Arven finds Earthen Vessel, Vessel discards a separate held Dragon
    // payload, searches the missing Grass, and the manual attachment completes GGF.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dialga-GX payload witness: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, search, Bench, Ability, Supporter, attachment procedure:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-complete-route contracts:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2271
    if (excluded_from_cost != Card::MysteriousTreasure ||
        scenario_.dci != DciProfile::StrictJit || !prizes_known() ||
        item_locked() || !supporter_allowed() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        !need_energy() || !need_payload() || need_regi() || need_vstar() ||
        need_active_vstar() || grass_needed() != 1 || fire_needed() != 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoV) == 0 || bench_space() <= 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        hand_count(Card::Arven) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    const bool separate_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), [](const Card card) {
          return is_payload(card);
        });
    return separate_payload;
  }

  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard_issue2271_original(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    if (ordinary) return ordinary;

    // Ordinary DCI always wins when it already has a lower-cost legal discard.
    // Only an otherwise blocked exact #2271 completion may spend Regidrago V.
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2271
    if (issue_2271_surplus_regidrago_v_treasure_route_available(
            excluded_from_cost)) {
      return Card::RegidragoV;
    }
    return std::nullopt;
  }

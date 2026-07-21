  std::optional<Card> quick_ball_dialga_vessel_first_cost() const {
    if (scenario_.dci != DciProfile::StrictJit || item_locked() || !deck_seen_ ||
        !can_play_payload_this_turn() || !need_payload() || !active_is_vstar() ||
        need_energy() || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::DialgaGX) == 0 ||
        (deck_count_after_search_started(Card::Grass) == 0 &&
         deck_count_after_search_started(Card::Fire) == 0)) {
      return std::nullopt;
    }

    // Quick Ball discards before searching Basic Dialga-GX. Earthen Vessel then
    // discards that fetched Dialga-GX and must still have a Basic Energy target.
    // In the setup goldfish Channeler has no modeled opposing attack effect, and
    // a held Basic Energy is dynamically surplus once the Active VSTAR already has
    // GGF. These route-conditioned costs are admitted only for this complete K1
    // two-Item chain and remain behind every ordinary lower-DCI discard category:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Opponent-action and dynamic-DCI bounds: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Earliest-route and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1237
    if (hand_count(Card::Channeler) > 0) return Card::Channeler;
    if (grass_needed() == 0 && hand_count(Card::Grass) > 0) return Card::Grass;
    if (fire_needed() == 0 && hand_count(Card::Fire) > 0) return Card::Fire;
    return std::nullopt;
  }

  bool has_payable_payload_outlet_after_costed_search(const Card excluded) {
    if (has_payable_payload_outlet_after_costed_search_issue1237_original(excluded)) {
      return true;
    }
    // Admission and execution share the same route-conditioned first cost. This
    // prevents the circular preflight from rejecting Quick Ball before it can
    // recognize the later Earthen Vessel outlet:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1237
    return excluded == Card::QuickBall &&
        quick_ball_dialga_vessel_first_cost().has_value();
  }

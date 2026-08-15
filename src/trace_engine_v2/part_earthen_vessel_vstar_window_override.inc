  bool hold_earthen_vessel_for_next_turn_latias_jit() const {
    if (!strict_payload_timing() || scenario_.locks != LockMode::None ||
        state_.turn >= scenario_.max_turn || !state_.manual_energy_used ||
        !need_energy() || !need_payload() || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || !in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::EarthenVessel) == 0 ||
        !payload_zone_contains(state_.hand)) {
      return false;
    }

    const Pokemon* setup_regi = target_regi();
    if (setup_regi == nullptr || setup_regi->card != Card::RegidragoVstar ||
        pays_apex_energy_cost(*setup_regi)) {
      return false;
    }

    // Reuse the canonical K0/K1 searchable-copy seam instead of re-encoding
    // hidden deck-versus-Prize visibility inside this route:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Canonical deck-count owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_knowledge.inc
    // Canonical payload owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
    const auto remaining_energy_count = [this](const Card card) {
      return searchable_copy_count(card);
    };
    const int grass_remaining = remaining_energy_count(Card::Grass);
    const int fire_remaining = remaining_energy_count(Card::Fire);
    const int basic_energy_remaining = grass_remaining + fire_remaining;
    const auto completing_basic = completing_basic_energy_for(
        *setup_regi, [&remaining_energy_count](const Card card) {
          return remaining_energy_count(card) > 0;
        }); // https://github.com/FlareZ123/pokemon-sims/issues/2425
    // Preserve Vessel and the held Dragon together when T2 has established GF and
    // Latias can promote the Benched VSTAR. On T3, Vessel discards that Dragon during
    // the strict-JIT ready turn, searches the final Grass, and leaves the Supporter
    // slot unused. Requiring two remaining Basic Energy cards keeps Vessel legally
    // playable after the intervening draw, even when that draw is the final Grass:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core attachment and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Strict JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1009
    // K0 hidden-inventory correction: https://github.com/FlareZ123/pokemon-sims/issues/1471
    return completing_basic.has_value() &&
        remaining_energy_count(*completing_basic) >= 1 &&
        basic_energy_remaining >= 2;
  }

  bool play_earthen_vessel(const bool permit_payload) {
    // Earthen Vessel may still search Basic Energy with an ordinary discard cost.
    // Its strict-JIT Dragon-payload cost is permitted only when the Regidrago VSTAR
    // card axis can finish this turn or a permitted payload is already established:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/954
    if (permit_payload && hold_earthen_vessel_for_next_turn_latias_jit()) {
      return false;
    }
    const bool payload_cost_permitted = permit_payload &&
        (payload_ready() || blender_vstar_card_axis_can_finish_this_turn());
    return play_earthen_vessel_vstar_window_original(payload_cost_permitted);
  }
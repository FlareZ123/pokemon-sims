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
    const bool payload_cost_permitted = permit_payload &&
        (payload_ready() || blender_vstar_card_axis_can_finish_this_turn());
    return play_earthen_vessel_vstar_window_original(payload_cost_permitted);
  }

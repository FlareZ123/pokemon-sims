  void play_items_until_stable_issue3990(const bool permit_payload) {
    bool powerglass_completes_apex_this_turn = false;
    if (permit_payload && state_.active &&
        state_.active->card == Card::RegidragoVstar &&
        state_.active->tool == Tool::None &&
        hand_count(Card::Powerglass) > 0 &&
        !pays_apex_energy_cost(*state_.active)) {
      for (const Card basic : {Card::Grass, Card::Fire}) {
        if (count_of(state_.discard, basic) == 0) continue;
        Pokemon projected = *state_.active;
        if (attach_energy_card(projected, basic) &&
            pays_apex_energy_cost(projected)) {
          powerglass_completes_apex_this_turn = true;
          break;
        }
      }
    }

    // A legal Powerglass attachment belongs before the strict-JIT Item fixed point
    // when its end-of-turn Basic Energy completes Apex on the already-Active VSTAR.
    // This lets a recovered Earthen Vessel discard the current-turn Dragon payload
    // after Legacy Star instead of banking the Vessel before the Tool is visible.
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Item, Tool, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Earliest-complete route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3990
    if (powerglass_completes_apex_this_turn) attach_powerglass();

    play_items_until_stable_issue3545(permit_payload);
  }

  bool issue_1393_held_crispin_completion_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        !deck_seen_ || !prizes_known() || !supporter_allowed() ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::Gladion) == 0 ||
        !need_energy() || !need_payload()) {
      return false;
    }

    const int grass = grass_needed();
    const int fire = fire_needed();
    if (grass > 1 || fire > 1) return false;

    const bool grass_available =
        deck_count_after_search_started(Card::Grass) > 0;
    const bool fire_available =
        deck_count_after_search_started(Card::Fire) > 0;
    const bool crispin_completes_energy = grass + fire == 2
        ? grass_available && fire_available
        : (grass == 1 ? grass_available : fire_available);
    const bool payload_outlet = has_live_blender_payload_line() ||
        has_live_one_discard_hand_payload_line();

    // In this public K1 turn, Crispin completes at most one missing Grass and one
    // missing Fire through its searched attachment plus the unused manual
    // attachment. The prior-turn Active Regidrago V can then evolve from the held
    // VSTAR card, while a held Blender or payable one-discard Item establishes the
    // same-turn Dragon payload. Gladion would consume the only Supporter play and
    // merely exchange a Prize card, so it is the slower and less resource-efficient
    // route:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, attachment, evolution, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, current-turn completion, resource preservation, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1393
    return crispin_completes_energy && payload_outlet;
  }

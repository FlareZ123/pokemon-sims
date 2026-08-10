  bool issue_1393_held_crispin_completion_available() const {
    // A resolved full Prize inspection establishes K1 without a deck search: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        !prizes_known() || !supporter_allowed() ||
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

    const std::array<Card, 2> basic_energies{Card::Grass, Card::Fire};
    const bool crispin_completes_energy = std::any_of(
        basic_energies.begin(), basic_energies.end(), [this](const Card energy) {
          if (deck_count_after_search_started(energy) == 0) return false;
          Pokemon projected = *state_.active;
          return attach_energy_card(projected, energy) &&
              pays_apex_energy_cost(projected);
        });
    const bool payload_outlet = has_live_blender_payload_line() ||
        has_live_one_discard_hand_payload_line();

    // If Crispin finds only one Basic Energy, that Energy goes to hand and Crispin
    // cannot attach it by effect. This route is still complete because the manual
    // attachment is unused: the searched Basic can be manually attached after the
    // prior-turn Regidrago V evolves. Projecting each available physical Basic
    // therefore models both the one-found legal sequence and the ordinary two-type
    // Crispin sequence without adding DDE's alternative type deficits together.
    // Official Crispin one-found ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, attachment, evolution, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // DDE semantic-readiness contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // K1, current-turn completion, resource preservation, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Original helper route: https://github.com/FlareZ123/pokemon-sims/issues/1918
    // Confirmed DDE migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2445
    return crispin_completes_energy && payload_outlet;
  }
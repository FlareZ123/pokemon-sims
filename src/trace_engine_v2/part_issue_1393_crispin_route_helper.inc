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

    const auto crispin_completes_energy = [this]() {
      const bool grass_available = deck_count_after_search_started(Card::Grass) > 0;
      const bool fire_available = deck_count_after_search_started(Card::Fire) > 0;
      if (!grass_available && !fire_available) return false;

      if (grass_available != fire_available) {
        // Official ruling: if Crispin finds only one Basic Energy, that card must
        // go to hand and cannot be attached by Crispin. This helper already requires
        // the turn's manual attachment to be unused, so project that legal manual
        // attachment from hand instead. DDE + either Basic then pays Apex Dragon:
        // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2445
        Pokemon after_manual = *state_.active;
        const Card sole_basic = grass_available ? Card::Grass : Card::Fire;
        return attach_energy_card(after_manual, sole_basic) &&
            pays_apex_energy_cost(after_manual);
      }

      for (const Card crispin_attachment : {Card::Grass, Card::Fire}) {
        Pokemon after_crispin = *state_.active;
        if (!attach_energy_card(after_crispin, crispin_attachment)) continue;
        if (pays_apex_energy_cost(after_crispin)) return true;

        // With both different Basic types found, Crispin attaches one and puts the
        // other into hand. The still-unused manual attachment may then provide that
        // other Basic when the Basic-only route needs both physical cards:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
        // Core one-manual-attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        const Card manual_basic = crispin_attachment == Card::Grass
            ? Card::Fire : Card::Grass;
        Pokemon after_manual = after_crispin;
        if (attach_energy_card(after_manual, manual_basic) &&
            pays_apex_energy_cost(after_manual)) {
          return true;
        }
      }
      return false;
    }();

    const bool payload_outlet = has_live_blender_payload_line() ||
        has_live_one_discard_hand_payload_line();

    // In this public K1 turn, held Crispin must produce a current-turn semantic
    // Apex completion through a legal searched-card disposition plus the unused
    // manual attachment when required. The prior-turn Active Regidrago V can then
    // evolve from the held VSTAR card, while a held Blender or payable one-discard
    // Item establishes the same-turn Dragon payload. Gladion would consume the only
    // Supporter play and merely exchange a Prize card, so it is the slower and less
    // resource-efficient route:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, attachment, evolution, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, current-turn completion, resource preservation, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed route contract: https://github.com/FlareZ123/pokemon-sims/issues/1918
    // Confirmed refined DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/2445
    return crispin_completes_energy && payload_outlet;
  }

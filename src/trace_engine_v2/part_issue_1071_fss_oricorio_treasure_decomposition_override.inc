  bool fss_latias_vessel_promotion_route_available() const {
    const bool latias_ability_available =
        scenario_.locks != LockMode::FullRuleBoxAbility &&
        scenario_.locks != LockMode::FullCombined;
    if (!deck_seen_ || scenario_.dci != DciProfile::StrictJit || item_locked() ||
        state_.manual_energy_used || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || bench_space() <= 0 ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        !latias_ability_available || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      const int missing_grass = std::max(0, 2 - pokemon.grass);
      const int missing_fire = std::max(0, 1 - pokemon.fire);
      if (missing_grass + missing_fire != 1) continue;

      const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
      if (deck_count_after_search_started(needed_energy) == 0) continue;

      // Star Alchemy must search Latias ex for the unresolved Active-position axis.
      // The held prior-turn V evolves, Earthen Vessel discards the held Dragon and
      // searches the final Basic Energy, the manual attachment completes GGF, and
      // Skyliner gives the Basic Active free Retreat during the same turn:
      // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Trainer, evolution, attachment, Ability, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1068
      return true;
    }
    return false;
  }

  bool fss_oricorio_with_held_treasure_vstar_route_available() const {
    if (!deck_seen_ || item_locked() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.turn >= scenario_.max_turn || bench_space() <= 0 ||
        in_play(Card::Oricorio) || hand_count(Card::Oricorio) > 0 ||
        deck_count_after_search_started(Card::Oricorio) == 0 ||
        !ability_available_for_pokemon(Card::Oricorio) ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        !payload_might_be_in_deck()) {
      return false;
    }

    const int redundant_payloads = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    if (redundant_payloads < 2) return false;

    const int missing_grass = std::max(
        0, 2 - state_.active->grass - hand_count(Card::Grass));
    const int missing_fire = std::max(
        0, 1 - state_.active->fire - hand_count(Card::Fire));
    if (missing_grass + missing_fire == 0 ||
        missing_grass + missing_fire > 2) {
      return false;
    }
    if (missing_grass > deck_count_after_search_started(Card::Grass) ||
        missing_fire > deck_count_after_search_started(Card::Fire)) {
      return false;
    }

    // Delegate the VSTAR axis to the held Mysterious Treasure, whose printed cost can
    // use one of two redundant held Dragon payloads. Star Alchemy is then free to find
    // Oricorio, Vital Dance searches only the Basic Energy not already held, and the
    // manual attachments across the available turns establish GGF. Held Professor
    // Burnet remains available for a deck-resident strict-JIT payload:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Oricorio and Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Trainer, Bench, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest connector decomposition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Redundant Dragon cost precedent: https://github.com/FlareZ123/pokemon-sims/issues/925
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1071
    return true;
  }

  bool fss_crispin_scheduled_turn_two_route_available() const {
    if (!deck_seen_ || scenario_.dci == DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::TurnTwoItem || state_.turn != 1 ||
        scenario_.going_first || scenario_.max_turn < 2 ||
        !supporter_allowed() || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        !payload_might_be_in_deck()) {
      return false;
    }

    for (const Card crispin_attachment : {Card::Grass, Card::Fire}) {
      int grass = state_.active->grass;
      int fire = state_.active->fire;
      int held_grass = hand_count(Card::Grass);
      int held_fire = hand_count(Card::Fire);

      if (crispin_attachment == Card::Grass) {
        ++grass;
        ++held_fire;
      } else {
        ++fire;
        ++held_grass;
      }

      const int remaining_grass = std::max(0, 2 - grass);
      const int remaining_fire = std::max(0, 1 - fire);
      const int remaining_attachments = remaining_grass + remaining_fire;
      if (remaining_attachments <= 2 && held_grass >= remaining_grass &&
          held_fire >= remaining_fire) {
        // Crispin searches different Basic Energy types, attaches one, and puts the
        // other into hand. The current and next legal manual attachments then finish
        // GGF before the scheduled T2 Item lock, while the held Burnet remains free to
        // establish the strict-JIT payload on the ready turn. This deterministic route
        // outranks Oricorio plus Celestial Roar's unknown top-three result:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
        // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
        // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Supporter, attachment, attack, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1085
        return true;
      }
    }
    return false;
  }

  Card fss_target_after_search_started() const {
    if (fss_latias_vessel_promotion_route_available()) {
      // The constrained Latias route solves Active position, Energy, Evolution, and
      // ready-turn payload together, so it outranks any delegated single-axis target:
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1068
      return Card::LatiasEx;
    }

    if (fss_crispin_scheduled_turn_two_route_available()) return Card::Crispin;

    const Card original = fss_target_after_search_started_issue1071_original();
    if (original == Card::RegidragoVstar &&
        fss_oricorio_with_held_treasure_vstar_route_available()) {
      return Card::Oricorio;
    }
    return original;
  }

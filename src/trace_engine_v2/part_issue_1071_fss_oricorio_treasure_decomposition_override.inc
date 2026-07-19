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
        !payload_might_be_in_deck() ||
        !is_legal_cost_available(false, true)) {
      return false;
    }

    const int redundant_payloads = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    if (redundant_payloads < 2) return false;

    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
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
    // Oricorio, Vital Dance searches the two-Energy package, and the unused manual
    // attachments across this turn and the legal evolution turn establish GGF. Held
    // Professor Burnet remains available for a deck-resident strict-JIT payload:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Oricorio and Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Trainer, Bench, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest connector decomposition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1071
    return true;
  }

  Card fss_target_after_search_started() const {
    const Card original = fss_target_after_search_started_issue1071_original();
    if (original == Card::RegidragoVstar &&
        fss_oricorio_with_held_treasure_vstar_route_available()) {
      return Card::Oricorio;
    }
    return original;
  }

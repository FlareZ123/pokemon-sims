  bool late_steven_active_vstar_crispin_treasure_route_available() const {
    const bool same_ready_turn_jit =
        scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::MatchupFlexJit;

    // The public state determines this continuation. Steven banks the future setup
    // pieces and ends the current turn; next turn Crispin, the unused attachment,
    // held Mysterious Treasure, and the held Dragon can finish Energy and payload.
    // None of these printed effects imposes an absolute-turn, seat, or StrictJit-only
    // condition. Rule Box Ability lock leaves the Trainer-only continuation legal.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1/JIT/lock/priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
    if (!same_ready_turn_jit || state_.turn < 2 ||
        scenario_.max_turn < state_.turn + 1 || !deck_seen_ ||
        !supporter_allowed() || state_.supporter_used || item_locked() ||
        state_.manual_energy_used || state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        state_.active->grass + state_.active->fire != 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        payload_cards_in_hand_count() == 0 ||
        deck_count_after_search_started(Card::BrilliantBlender) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0) {
      return false;
    }

    // Preserve the established current-route projection and Energy Switch/topdeck
    // dominance test. Only the semantic admission boundary changes here.
    // Energy Switch: https://api.pokemontcg.io/v2/cards/sv1-173
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
    const auto has_weighted_out = [this]() {
      for (const Card card : {Card::BrilliantBlender, Card::EnergySwitch,
                              Card::Fire, Card::Grass, Card::EarthenVessel,
                              Card::Crispin, Card::StevensResolve}) {
        if (weighted_deck_out_count(card) > 0) return true;
      }
      return false;
    };

    auto copy = *this;
    bool target_available = true;
    for (const Card card : {Card::BrilliantBlender, Card::Grass, Card::Fire}) {
      if (!copy.move_prize_to_hand(card)) {
        target_available = false;
        break;
      }
    }
    if (target_available && !copy.play_mysterious_treasure(Card::MegaDragonite))
      target_available = false;
    if (target_available &&
        !copy.discard_from_hand(Card::MegaDragonite,
                                "projected active-VSTAR Steven route",
                                "P-DCI-01; P-COMPRESS-01"))
      target_available = false;
    if (target_available && !copy.play_crispin()) target_available = false;

    const bool direct_ready = target_available && !copy.need_energy() &&
        !copy.need_payload() && !copy.ready();
    return !direct_ready ||
        deck_count_after_search_started(Card::EnergySwitch) == 0 ||
        hand_count(Card::EnergySwitch) > 0 || has_weighted_out();
  }

  bool issue_1236_pre_steven_vessel_route_available() const {
    if (scenario_.dci != DciProfile::MatchupFlexJit || state_.turn != 1 ||
        scenario_.going_first || scenario_.locks != LockMode::None ||
        !supporter_allowed() || state_.manual_energy_used || state_.supporter_used ||
        state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->tool != Tool::None || !need_vstar() || !need_energy() ||
        hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RoseannesBackup) == 0 ||
        hand_count(Card::ForestSealStone) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::StevensResolve) > 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Earthen Vessel uses one duplicate Mysterious Treasure as its distinct cost.
    // The surviving Treasure then uses Roseanne's Backup to find Regidrago VSTAR.
    // Forest Seal Stone finds Steven's Resolve after the T1 Grass attachment, and
    // Steven obtains Crispin plus Brilliant Blender for the deterministic T2 finish:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Tool, VSTAR Power, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1236
    return might_be_unseen(Card::RegidragoVstar) &&
        might_be_unseen(Card::StevensResolve) &&
        might_be_unseen(Card::Crispin) &&
        might_be_unseen(Card::BrilliantBlender) &&
        might_be_unseen(Card::Grass) && might_be_unseen(Card::Fire);
  }

  std::optional<Card> choose_discard(const bool permit_payload,
                                     const bool flex_fodder,
                                     const bool allow_heavy_ball = true,
                                     const std::optional<Card> excluded_from_cost = std::nullopt,
                                     const bool allow_redundant_payload = false) const {
    if (excluded_from_cost && *excluded_from_cost == Card::EarthenVessel &&
        issue_1236_pre_steven_vessel_route_available()) {
      // Preserve Roseanne's Backup for the surviving Mysterious Treasure cost.
      // A duplicate Treasure is legally another card for Earthen Vessel and has
      // higher route-conditioned DCI in this exact two-cost T1 sequence:
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1236
      return Card::MysteriousTreasure;
    }
    return choose_discard_issue1236_original(
        permit_payload, flex_fodder, allow_heavy_ball,
        excluded_from_cost, allow_redundant_payload);
  }

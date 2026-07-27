  bool issue_1412_direct_fss_fire_route_visible() const {
    const Pokemon* regi = state_.active ? &*state_.active : nullptr;
    const bool active_is_regi = regi != nullptr &&
        (regi->card == Card::RegidragoV ||
         regi->card == Card::RegidragoVstar);
    const bool active_evolution_axis_complete = active_is_regi &&
        (regi->card == Card::RegidragoVstar ||
         (regi->card == Card::RegidragoV &&
          regi->entered_turn < state_.turn &&
          hand_count(Card::RegidragoVstar) > 0));
    const auto can_hold_fss = [](const Pokemon& pokemon) {
      return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
    };
    const bool held_fss_attachable =
        hand_count(Card::ForestSealStone) > 0 && !state_.vstar_power_used &&
        ((state_.active && can_hold_fss(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), can_hold_fss));
    const bool fire_held = hand_count(Card::Fire) > 0;
    const bool fss_fire_available = might_be_unseen(Card::Fire) &&
        (secret_box_has_live_fss_holder() || held_fss_attachable);

    // After Exploding Energy supplies GG, this public graph has one unresolved
    // Energy axis. Forest Seal Stone searches Fire, the unused manual attachment
    // pays it, and the held evolution-eligible VSTAR finishes the Active attacker:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Tool, Ability, attachment, and evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0/K1 boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1412
    return outcome_.used_exploding_energy && secret_box_combo_enabled() &&
        !state_.turn_ended && !rule_box_abilities_locked() &&
        !state_.manual_energy_used && regi != nullptr && regi->grass >= 2 &&
        regi->fire == 0 && payload_ready() &&
        active_evolution_axis_complete && (fire_held || fss_fire_available);
  }

  bool issue_1646_vessel_burnet_finish_visible() const {
    // The Active VSTAR is already at GF, so one searchable Grass and the unused
    // manual attachment complete the Energy axis. Professor Burnet is the paid
    // current-turn payload outlet. In this exact public K1 graph, Quick Ball is a
    // dominated connector: using it for Tapu Lele-GX would consume another Bench
    // slot and force Brilliant Blender, while paying Vessel with Quick Ball keeps
    // the singleton ACE SPEC and completes both remaining axes this turn:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, attachment, search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, strict JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1646
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn == 3 &&
        prizes_known() && supporter_allowed() && !state_.manual_energy_used &&
        state_.active && state_.active->card == Card::RegidragoVstar &&
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        need_payload() && hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        payload_might_be_in_deck();
  }

  bool play_issue_1646_vessel_burnet_finish() {
    if (!issue_1646_vessel_burnet_finish_visible()) return false;
    if (item_locked() || !deck_search_available()) return false;

    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error("Issue 1646 Earthen Vessel disappeared");
    }
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(Card::QuickBall, "Earthen Vessel cost", "R-EV-01");
    record_deck_search_knowledge("Earthen Vessel");

    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue 1646 Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded the dominated Quick Ball and searched Grass Energy for the held Burnet finish.");
    return true;
  }

  bool play_earthen_vessel(const bool permit_payload) {
    // Earthen Vessel and its discard cost resolve no missing axis once the exact
    // Forest Seal Stone route is public. Preserve Vessel and its prospective cost
    // until the existing FSS, manual-attachment, and evolution sequence resolves:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dynamic DCI and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1412
    if (issue_1412_direct_fss_fire_route_visible()) return false;
    if (play_issue_1646_vessel_burnet_finish()) return true;
    return play_earthen_vessel_issue1412_original(permit_payload);
  }

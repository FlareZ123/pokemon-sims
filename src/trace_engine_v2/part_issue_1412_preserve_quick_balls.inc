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

  std::optional<Card> issue_1866_vessel_burnet_cost() const {
    // Once Active Regidrago VSTAR is one manual attachment from GGF and held
    // Professor Burnet deterministically supplies the current-turn payload,
    // these visible cards have no remaining setup role on the complete route.
    // Preserve Basic Energy, Dragon payloads, recovery, ACE SPEC, and other live
    // connectors while selecting the narrow route-replaced Vessel cost:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
    // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
    // Dynamic DCI and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
    for (const Card card : {
             Card::QuickBall,
             Card::MysteriousTreasure,
             Card::ErikasInvitation,
             Card::Guzma,
             Card::Channeler,
             Card::FieldBlower,
             Card::ChaoticSwell,
             Card::PathToPeak,
             Card::TeamYellsCheer,
             Card::Powerglass,
         }) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1646_vessel_burnet_finish_visible() const {
    const Pokemon* active = state_.active ? &*state_.active : nullptr;
    if (active == nullptr || active->card != Card::RegidragoVstar) return false;

    const bool missing_energy_in_deck =
        completing_basic_energy_for(*active, [this](const Card card) {
          return deck_count_after_search_started(card) > 0;
        }).has_value();
    const bool exactly_one_energy_missing = missing_energy_in_deck;
    // DDE-only is one physical Basic attachment from Apex even though either
    // Basic type can complete it: https://github.com/FlareZ123/pokemon-sims/issues/2368

    // Earthen Vessel's printed cost and search resolve the only missing GGF
    // component, Professor Burnet independently establishes a current-turn
    // Dragon payload, and the unused manual attachment completes readiness.
    // The route applies to either final Basic Energy orientation and does not
    // require Brilliant Blender. A held Dragon remains the stronger direct
    // Vessel cost and therefore blocks this paid-Supporter route. K1 and the
    // narrow cost selector prevent hidden information use and preserve every
    // still-live connector:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, search, discard, shuffle, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict JIT, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1646
    // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn >= 3 &&
        state_.turn <= scenario_.max_turn && prizes_known() &&
        supporter_allowed() && !state_.manual_energy_used &&
        exactly_one_energy_missing && missing_energy_in_deck &&
        need_payload() &&
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        payload_might_be_in_deck() &&
        issue_1866_vessel_burnet_cost().has_value();
  }

  bool play_issue_1646_vessel_burnet_finish() {
    if (!issue_1646_vessel_burnet_finish_visible()) return false;
    if (item_locked() || !deck_search_available()) return false;

    const Card cost = *issue_1866_vessel_burnet_cost();
    const Card missing_energy =
        state_.active->grass < 2 ? Card::Grass : Card::Fire;

    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error("Issue 1866 Earthen Vessel disappeared");
    }
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(cost, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue 1866 Vessel cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel");

    if (!move_deck_to_hand(missing_energy)) {
      throw std::logic_error("Issue 1866 Basic Energy target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(cost)) +
              " as the route-replaced cost and searched " +
              std::string(name(missing_energy)) +
              " for the held Professor Burnet finish.");

    // Resolve the proven Supporter half immediately so Brilliant Blender cannot
    // preempt the deterministic Burnet payload before the manual attachment:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Supporter timing and action order: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1866
    if (!play_professor_burnet()) {
      throw std::logic_error("Issue 1866 Professor Burnet route disappeared");
    }
    return true;
  }
  bool play_issue_1672_vessel_payload_finish() {
    const Pokemon* setup_regi = target_regi();
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    if (scenario_.dci != DciProfile::MatchupFlexJit ||
        scenario_.locks != LockMode::None || state_.turn != scenario_.max_turn ||
        !prizes_known() || item_locked() || !state_.supporter_used ||
        state_.manual_energy_used || state_.retreat_used ||
        !state_.vstar_power_used || !state_.active ||
        !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::LatiasEx) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        payload == state_.hand.end() || setup_regi == nullptr ||
        setup_regi->card != Card::RegidragoV ||
        setup_regi->entered_turn >= state_.turn ||
        setup_regi->tool != Tool::ForestSealStone ||
        setup_regi->grass != 1 || setup_regi->fire < 1 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    const Card discarded_payload = *payload;
    // Star Alchemy has already searched Latias ex. Earthen Vessel must now use the
    // held Dragon as its mandatory cost, because that single payment establishes the
    // matchup-flex current-turn payload while Vessel searches the third GGF Energy.
    // The held VSTAR and Skyliner route then complete evolution and promotion:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Dialga-GX Dragon payload: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, search, attachment, evolution, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI, strict current-turn payload, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1672
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(discarded_payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1672 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel");
    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-1672 Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(discarded_payload)) +
              " as the current-turn payload and searched Grass Energy for the Latias promotion finish.");
    return true;
  }

  bool play_issue_1868_vessel_payload_finish() {
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    if ((scenario_.dci != DciProfile::StrictJit &&
         scenario_.dci != DciProfile::MatchupFlexJit) ||
        scenario_.locks != LockMode::None || !prizes_known() || item_locked() ||
        !deck_search_available() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        state_.active->grass != 1 || state_.active->fire < 1 ||
        !need_payload() || hand_count(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        payload == state_.hand.end()) {
      return false;
    }

    const Card discarded_payload = *payload;
    // Earthen Vessel discards another card before searching Basic Energy. In
    // this public K1 state, its Grass search plus the unused manual attachment
    // completes GGF, so the held Dragon becomes DCI 1 because that same cost
    // supplies the required strict/matchup-flex current-turn Apex Dragon payload:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex payload witness: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, current-turn JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original matchup-flex bug: https://github.com/FlareZ123/pokemon-sims/issues/1868
    // Confirmed strict regression: https://github.com/FlareZ123/pokemon-sims/issues/2224
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(discarded_payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1868/2224 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1868/2224 route");
    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM; P-JIT-01",
                          "Earthen Vessel");
    return true;
  }

  bool play_issue_1898_pineco_vessel_payload_finish() {
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    if ((scenario_.dci != DciProfile::StrictJit &&
         scenario_.dci != DciProfile::MatchupFlexJit) ||
        scenario_.locks != LockMode::None || !prizes_known() || item_locked() ||
        !deck_search_available() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        state_.active->grass < 2 || state_.active->fire != 0 ||
        !need_payload() || hand_count(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        payload == state_.hand.end()) {
      return false;
    }

    const Card discarded_payload = *payload;
    // Earthen Vessel requires another card from hand as its printed cost. In
    // this public K1 state, the unused manual attachment and a searchable Fire
    // Energy complete Apex Dragon's GGF cost, so the held Dragon is DCI 1: the
    // same Vessel payment establishes the required strict/matchup-flex current-turn
    // payload while preserving higher-value live resources.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Guzma discrete-value witness: https://api.pokemontcg.io/v2/cards/sm3-115
    // Official Item-cost, discard, search, and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, discrete value, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed matchup-flex bug: https://github.com/FlareZ123/pokemon-sims/issues/1898
    // Confirmed strict-JIT GG regression: https://github.com/FlareZ123/pokemon-sims/issues/2231
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(discarded_payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1898/2231 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1898/2231 route");
    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM; P-JIT-01",
                          "Earthen Vessel");
    return true;
  }

  bool play_earthen_vessel(const bool permit_payload) {
    if (play_issue_1898_pineco_vessel_payload_finish()) return true;
    if (play_issue_1868_vessel_payload_finish()) return true;
    // Legacy Star may recover Earthen Vessel on the reported T2 `GF` state
    // whose manual Energy attachment is already spent. Preserve the Item until
    // T3, when its Dragon cost and searched Grass complete strict-JIT together:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Item, discard, search, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    if (legacy_star_delayed_vessel_route()) return false;
    // Earthen Vessel and its discard cost resolve no missing axis once the exact
    // Forest Seal Stone route is public. Preserve Vessel and its prospective cost
    // until the existing FSS, manual-attachment, and evolution sequence resolves:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dynamic DCI and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1412
    if (issue_1412_direct_fss_fire_route_visible()) return false;
    if (play_issue_1672_vessel_payload_finish()) return true;
    if (play_issue_1646_vessel_burnet_finish()) return true;
    if (play_issue_2437_direct_dde_vessel_energy_route()) return true;
    return play_earthen_vessel_issue1412_original(permit_payload);
  }
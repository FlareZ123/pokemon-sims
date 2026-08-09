  std::optional<Card> issue_2437_apex_completing_basic(
      const Pokemon& target, const bool grass_available,
      const bool fire_available) const {
    // A physical Basic Energy is one attachment. With DDE already attached to a
    // Dragon, either Basic type can be the single final card when the semantic
    // projection pays Apex Dragon; do not add typed alternatives as two deficits:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual Energy attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // DDE enhancement contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    if (!is_dragon(target.card) || pays_apex_energy_cost(target)) {
      return std::nullopt;
    }
    for (const auto [basic, available] : {
             std::pair{Card::Grass, grass_available},
             std::pair{Card::Fire, fire_available},
         }) {
      if (!available) continue;
      Pokemon projected = target;
      if (attach_energy_card(projected, basic) &&
          pays_apex_energy_cost(projected)) {
        return basic;
      }
    }
    return std::nullopt;
  }

  std::optional<Card> issue_2437_apex_completing_basic_from_known_deck(
      const Pokemon& target) const {
    return issue_2437_apex_completing_basic(
        target, deck_count_after_search_started(Card::Grass) > 0,
        deck_count_after_search_started(Card::Fire) > 0);
  }

  std::optional<Card> issue_2437_apex_completing_basic_from_possible_deck(
      const Pokemon& target) const {
    return issue_2437_apex_completing_basic(
        target, might_be_unseen(Card::Grass), might_be_unseen(Card::Fire));
  }

  std::optional<Card> issue_2437_direct_vessel_cost() const {
    const auto ordinary = choose_discard(false, true, true, Card::EarthenVessel);
    if (ordinary.has_value()) return ordinary;

    // In the no-lock goldfish state, Field Blower has no current setup target.
    // Spending it lets Vessel take the single Basic that completes a DDE-backed
    // Apex line while preserving Quick Ball/Tapu/Crispin for later axes:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Earliest-route / dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed integration bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    if (scenario_.locks == LockMode::None && state_.stadium == Stadium::None &&
        hand_count(Card::FieldBlower) > 0) {
      return Card::FieldBlower;
    }
    return std::nullopt;
  }

  bool play_issue_2437_direct_dde_vessel_energy_route() {
    const Pokemon* target = target_regi();
    const auto completing = target != nullptr
        ? issue_2437_apex_completing_basic_from_known_deck(*target)
        : std::optional<Card>{};
    const auto cost = issue_2437_direct_vessel_cost();
    if (item_locked() || !deck_search_available() || state_.manual_energy_used ||
        target == nullptr || target->double_dragon <= 0 ||
        pays_apex_energy_cost(*target) || !completing.has_value() ||
        hand_count(Card::EarthenVessel) == 0 || !cost.has_value()) {
      return false;
    }

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*cost, "Earthen Vessel issue-2437 direct cost",
                           "R-EV-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-2437 direct Vessel cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-2437 direct route");
    if (!move_deck_to_hand(*completing)) {
      throw std::logic_error("Issue-2437 direct Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Spent a route-replaced cost and searched " +
              std::string(name(*completing)) +
              " as the one-attachment DDE Apex completion.");
    return true;
  }

  bool issue_1646_vessel_burnet_finish_visible() const {
    const Pokemon* active = state_.active ? &*state_.active : nullptr;
    if (active == nullptr || active->card != Card::RegidragoVstar) return false;

    const auto missing_energy =
        issue_2437_apex_completing_basic_from_known_deck(*active);

    // Vessel searches exactly one legal completing Basic for this projection;
    // Professor Burnet independently establishes the same-turn Dragon payload.
    // DDE can make Grass and Fire alternative one-card completions, so the route
    // is keyed to semantic payment rather than raw Basic counters:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, strict JIT, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1646
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn >= 3 &&
        state_.turn <= scenario_.max_turn && prizes_known() &&
        supporter_allowed() && !state_.manual_energy_used &&
        missing_energy.has_value() && need_payload() &&
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
    const auto missing_energy =
        issue_2437_apex_completing_basic_from_known_deck(*state_.active);
    if (!missing_energy) {
      throw std::logic_error("Issue-2437 Vessel Basic target disappeared");
    }

    // The target is the specific Basic whose one-card projection pays Apex; this
    // permits either Grass or Fire beside DDE while preserving the legacy GF/GG
    // no-DDE orientations:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error("Issue-2437 Earthen Vessel disappeared");
    }
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(cost, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-2437 Vessel cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel");
    if (!move_deck_to_hand(*missing_energy)) {
      throw std::logic_error("Issue-2437 Basic Energy target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(cost)) +
              " as the route-replaced cost and searched " +
              std::string(name(*missing_energy)) +
              " for the held Professor Burnet finish.");

    // Professor Burnet is the proven payload half of this compressed route:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Official one-Supporter-per-turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    if (!play_professor_burnet()) {
      throw std::logic_error("Issue-2437 Professor Burnet route disappeared");
    }
    return true;
  }

  bool play_issue_1672_vessel_payload_finish() {
    const Pokemon* setup_regi = target_regi();
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const auto missing_energy = setup_regi
        ? issue_2437_apex_completing_basic_from_known_deck(*setup_regi)
        : std::optional<Card>{};
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
        !missing_energy.has_value()) {
      return false;
    }

    const Card discarded_payload = *payload;
    // Vessel's mandatory hand discard supplies the current-turn Dragon payload;
    // the one searched Basic selected by semantic projection completes Apex after
    // the unused manual attachment, including DDE + Grass and DDE + Fire states:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, attachment, evolution, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1672
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(discarded_payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-2437 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel");
    if (!move_deck_to_hand(*missing_energy)) {
      throw std::logic_error("Issue-2437 Latias-route Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(discarded_payload)) +
              " as the current-turn payload and searched " +
              std::string(name(*missing_energy)) +
              " for the Latias promotion finish.");
    return true;
  }

  bool play_issue_1868_vessel_payload_finish() {
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool dde_backed = state_.active && state_.active->double_dragon > 0;
    const bool canonical_grass_orientation = state_.active && !dde_backed &&
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        deck_count_after_search_started(Card::Grass) > 0;
    const auto missing_energy = dde_backed
        ? issue_2437_apex_completing_basic_from_known_deck(*state_.active)
        : (canonical_grass_orientation
               ? std::optional<Card>{Card::Grass}
               : std::optional<Card>{});
    // #1868/#2224 is the canonical GF -> Grass route. DDE-backed states may use
    // either Basic when semantic projection proves it completes Apex. Canonical
    // GG -> Fire remains with its dedicated missing-Fire route.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Canonical route boundaries: https://github.com/FlareZ123/pokemon-sims/issues/1868 https://github.com/FlareZ123/pokemon-sims/issues/2224 https://github.com/FlareZ123/pokemon-sims/issues/2231
    // DDE migration: https://github.com/FlareZ123/pokemon-sims/issues/2437
    if ((scenario_.dci != DciProfile::StrictJit &&
         scenario_.dci != DciProfile::MatchupFlexJit) ||
        scenario_.locks != LockMode::None || !prizes_known() || item_locked() ||
        !deck_search_available() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        !missing_energy.has_value() || !need_payload() ||
        hand_count(Card::EarthenVessel) == 0 ||
        payload == state_.hand.end()) {
      return false;
    }

    const Card discarded_payload = *payload;
    // Vessel can search up to two Basic Energy, so selecting the single Basic that
    // the semantic projection proves sufficient preserves unnecessary deck cards.
    // Its printed discard simultaneously establishes the current-turn payload:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original routes: https://github.com/FlareZ123/pokemon-sims/issues/1868 https://github.com/FlareZ123/pokemon-sims/issues/2224
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2437
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(discarded_payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-2437 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-2437 route");
    if (!move_deck_to_hand(*missing_energy)) {
      throw std::logic_error("Issue-2437 Active-VSTAR Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(discarded_payload)) +
              " as the current-turn payload and searched " +
              std::string(name(*missing_energy)) +
              " as the one-attachment Apex completion.");
    return true;
  }
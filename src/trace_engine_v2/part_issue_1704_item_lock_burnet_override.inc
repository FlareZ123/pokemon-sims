  bool issue_1704_prior_turn_pineco() const {
    const auto eligible = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::Pineco && pokemon.entered_turn < state_.turn;
    };
    return (state_.active && eligible(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), eligible);
  }

  bool issue_1704_t1_public_entry_state() const {
    return secret_box_combo_enabled() && state_.turn == 1 &&
        !scenario_.going_first && scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::TurnTwoItem && !item_locked() &&
        supporter_allowed() && !state_.manual_energy_used && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->fire == 0 && issue_1704_prior_turn_pineco() &&
        bench_space() > 0 && hand_count(Card::TapuLeleGX) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::StevensResolve) > 0 &&
        (hand_count(Card::ForestOfVitality) > 0 ||
         hand_count(Card::QuickBall) > 0) &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::ForretressEx);
  }

  bool play_issue_1704_t1_item_lock_burnet_route() {
    if (!issue_1704_t1_public_entry_state()) return false;

    // Wonder Tag is the first legal deck inspection. The route is selected only after
    // K1 proves Burnet, Forretress ex, Fire, two Grass Energy, and a Dragon payload
    // remain available. This avoids using hidden deck order as a pre-search oracle:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // K0/K1 and scheduled-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1704
    if (!bench_from_hand(Card::TapuLeleGX, false)) return false;
    record_deck_search_knowledge("Wonder Tag issue-1704 Item-lock route");

    const bool complete_route =
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        deck_count_after_search_started(Card::ForretressEx) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        payload_might_be_in_deck();

    Card wonder_tag_target = Card::ProfessorBurnet;
    if (!complete_route) {
      for (const Card fallback : {Card::Dawn, Card::Arven,
                                  Card::StevensResolve, Card::ProfessorBurnet}) {
        if (deck_count_after_search_started(fallback) > 0) {
          wonder_tag_target = fallback;
          break;
        }
      }
    }
    const bool supporter_found = move_deck_to_hand(wonder_tag_target);
    shuffle(state_.deck);
    trace("USE ABILITY", "R-TAPU-01; P-KNOWLEDGE-01",
          supporter_found
              ? "Wonder Tag searched " + std::string(name(wonder_tag_target)) + "."
              : "Wonder Tag found no Supporter for the scheduled Item-lock route.");
    if (!complete_route || !supporter_found ||
        wonder_tag_target != Card::ProfessorBurnet) {
      return false;
    }

    const Card vessel_cost = hand_count(Card::ForestOfVitality) > 0
        ? Card::ForestOfVitality
        : Card::QuickBall;
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(vessel_cost, "Earthen Vessel issue-1704 cost",
                           "R-EV-01; P-DCI-01")) {
      throw std::logic_error("Issue-1704 Vessel cost disappeared");
    }
    if (!move_deck_to_hand(Card::Fire)) {
      throw std::logic_error("Issue-1704 searchable Fire disappeared");
    }
    shuffle(state_.deck);
    // Prior-turn Pineco replaces Forest of Vitality, while Steven replaces Quick Ball's
    // missing Stage 1 search. Either exact cost therefore has a complete public route:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Dynamic DCI replacement proof: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1704
    trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-DCI-01",
          "Earthen Vessel discarded " + std::string(name(vessel_cost)) +
              " and searched Fire Energy before the scheduled Item lock.");

    remove_one(state_.hand, Card::Fire);
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          "Fire Energy manually to the prior-turn Active Regidrago V.");

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1704 Item-lock route");
    if (!move_deck_to_hand(Card::ForretressEx)) {
      throw std::logic_error("Issue-1704 searchable Forretress ex disappeared");
    }
    shuffle(state_.deck);
    // Steven may search up to three cards, so taking only Forretress ex preserves hand
    // value and avoids banking Items that the known T2 lock makes unusable:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Core Item, Supporter, attachment, evolution, Ability, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Scheduled Item-lock and Burnet bridge: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1704
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-FORRETRESS-01; P-COMPRESS-01",
          "Steven's Resolve searched only Forretress ex for the deterministic T2 Burnet route; the turn ended.");
    return true;
  }

  void run_secret_box_turn_issue1704() {
    if (play_issue_1704_t1_item_lock_burnet_route()) {
      trace("TURN END", "R-STEVEN-01; R-EV-01; P-AXIS-01", state_line());
      return;
    }
    run_secret_box_turn();
  }

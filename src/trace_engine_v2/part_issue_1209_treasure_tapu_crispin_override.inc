  bool issue_1209_t1_treasure_tapu_crispin_route_available() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 1 ||
        !scenario_.going_first || scenario_.locks != LockMode::None ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::LatiasEx ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        bench_space() < 2 || !need_regi() ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Grass) < 2 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Crispin) > 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool regi_available = deck_seen_
        ? deck_count_after_search_started(Card::RegidragoV) > 0
        : might_be_unseen(Card::RegidragoV);
    const bool tapu_available = deck_seen_
        ? deck_count_after_search_started(Card::TapuLeleGX) > 0
        : might_be_unseen(Card::TapuLeleGX);
    const bool crispin_available = deck_seen_
        ? deck_count_after_search_started(Card::Crispin) > 0
        : might_be_unseen(Card::Crispin);
    const bool grass_available = deck_seen_
        ? deck_count_after_search_started(Card::Grass) > 0
        : might_be_unseen(Card::Grass);
    const bool fire_available = deck_seen_
        ? deck_count_after_search_started(Card::Fire) > 0
        : might_be_unseen(Card::Fire);

    // One held Grass is a legal dynamic Quick Ball cost because the second Grass is
    // attached to the searched Regidrago V on turn one. On turn two, Mysterious
    // Treasure discards the held Dragon payload and searches Psychic Tapu Lele-GX;
    // Wonder Tag searches Crispin, whose two different Basic Energy targets supply
    // the remaining GGF components through its attachment and the manual attachment.
    // Latias ex supplies the zero-Retreat promotion after the prior-turn Basic evolves:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Ability, Supporter, attachment, evolution, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0/K1, dynamic DCI, earliest-route, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
    return regi_available && tapu_available && crispin_available &&
        grass_available && fire_available;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (play_quick_ball_issue1209_original(permit_payload)) return true;
    if (!issue_1209_t1_treasure_tapu_crispin_route_available()) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(Card::Grass, "Quick Ball cost", "R-QB-01");
    record_deck_search_knowledge("Quick Ball");

    Card selected = Card::RegidragoV;
    bool found = move_deck_to_hand(selected);
    if (!found) {
      selected = fallback_quick_ball_target_after_search_started();
      found = move_deck_to_hand(selected);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01",
          found ? "Spent one dynamically replaceable Grass Energy and searched a Basic Pokemon: " +
                      std::string(name(selected)) + "."
                : "Spent one dynamically replaceable Grass Energy; no Basic Pokemon remained after search.");
    return true;
  }

  bool issue_1209_t2_treasure_tapu_context_available() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 2 ||
        scenario_.locks != LockMode::None || !deck_seen_ ||
        !supporter_allowed() || state_.supporter_used ||
        hand_count(Card::Arven) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Crispin) > 0 || bench_space() == 0 ||
        !state_.active || state_.active->card != Card::LatiasEx ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        state_.retreat_used ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    return std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return (pokemon.card == Card::RegidragoV ||
                  pokemon.card == Card::RegidragoVstar) &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 1;
        });
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (play_mysterious_treasure_issue1209_original(permit_payload)) return true;
    if (!permit_payload ||
        !issue_1209_t2_treasure_tapu_context_available()) {
      return false;
    }

    const auto cost = choose_discard(
        true, true, true, Card::MysteriousTreasure);
    if (!cost || !is_payload(*cost)) return false;
    const Card discarded_payload = *cost;

    // This exact current-turn connector is source-bounded after the T1 Quick Ball
    // inspection. Mysterious Treasure may pay the Dragon payload now because the
    // searched Tapu Lele-GX obtains Crispin, and Crispin deterministically completes
    // the remaining GGF Energy axis before the same turn's ready-state check:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Ability, Supporter, attachment, evolution, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, earliest-route, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(discarded_payload,
                      "Mysterious Treasure cost", "R-MT-01; P-JIT-01");
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error(
          "projected issue-1209 Mysterious Treasure Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Discarded the current-turn Dragon payload and searched Tapu Lele-GX for the Crispin completion route.");
    return true;
  }

  bool issue_1209_t2_treasure_tapu_crispin_completion_available() const {
    if (!issue_1209_t2_treasure_tapu_context_available()) return false;

    Engine projected = *this;
    projected.trace_ = nullptr;
    if (!projected.play_mysterious_treasure(true)) return false;
    projected.play_basics_from_hand();
    if (projected.hand_count(Card::Crispin) == 0 ||
        !projected.play_crispin()) {
      return false;
    }
    projected.attach_manual();
    projected.evolve_best_regi();
    projected.retreat_to_benched_vstar_with_latias();

    return projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 &&
        projected.payload_ready();
  }

  void choose_supporter() {
    if (issue_1209_t2_treasure_tapu_crispin_completion_available()) {
      // Preserve the one Supporter play until Mysterious Treasure pays its current-turn
      // Dragon cost, searches Tapu Lele-GX, and Wonder Tag obtains Crispin. Spending
      // Arven first consumes the only Supporter play and loses the proven T2 finish:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core Supporter, Item, Ability, attachment, evolution, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest-route and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1209-treasure-tapu-crispin-before-arven", state_.turn,
            "HOLD SUPPORTER", "R-MT-01; R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
            "Retained the Supporter play for the proven Mysterious Treasure to Tapu Lele-GX to Crispin completion route.");
      }
      return;
    }
    choose_supporter_issue1209_original();
  }

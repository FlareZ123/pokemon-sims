  bool issue_1356_fss_treasure_energy_split_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        scenario_.max_turn < 3 || !deck_seen_ || state_.vstar_power_used ||
        !state_.manual_energy_used || item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool star_takes_fire =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    const bool star_takes_grass =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0;

    // Mysterious Treasure can pay the surplus off-type Energy and search the
    // missing Regidrago VSTAR. Star Alchemy is the costless connector for the
    // sole missing next-turn Basic Energy, while the second Treasure and held
    // Dragon remain the strict-JIT payload route:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, VSTAR Power, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
    return star_takes_fire || star_takes_grass;
  }

  bool use_fss() {
    if (!issue_1356_fss_treasure_energy_split_available()) {
      return use_fss_issue1356_original();
    }

    const Card target = state_.active->grass >= 2 ? Card::Fire : Card::Grass;
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    record_deck_search_knowledge("Star Alchemy");
    if (!move_deck_to_hand(target)) {
      throw std::logic_error("projected issue-1356 Basic Energy target disappeared");
    }
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01; P-COMPRESS-01",
          "Searched the missing next-turn Basic Energy while Mysterious Treasure retained the Regidrago VSTAR axis: " +
              std::string(name(target)) + ".");
    return true;
  }

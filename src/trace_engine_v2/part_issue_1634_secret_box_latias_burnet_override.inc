  bool issue_1634_secret_box_latias_burnet_axes_visible() const {
    const bool powered_benched_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 &&
              pokemon.fire >= 1;
        });

    // This is a K1-only direct-completion route. The Benched VSTAR already owns
    // the Pokémon and GGF axes, while Secret Box can use only its Item and
    // Supporter searches. Mysterious Treasure finds Latias ex, Professor Burnet
    // supplies the current-turn Dragon payload, and Skyliner supplies the free
    // retreat from a Basic Active. Optional Tool and Stadium searches may fail:
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, Bench, Ability, retreat, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, and shortest-complete-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1634
    return secret_box_combo_enabled() && deck_seen_ && !item_locked() &&
        supporter_allowed() && !state_.retreat_used && state_.active &&
        is_basic(state_.active->card) && bench_space() > 0 &&
        powered_benched_vstar && need_active_vstar() && !need_energy() &&
        need_payload() && can_play_payload_this_turn() &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        count_of(state_.deck, Card::MysteriousTreasure) > 0 &&
        count_of(state_.deck, Card::LatiasEx) > 0 &&
        count_of(state_.deck, Card::ProfessorBurnet) > 0 &&
        payload_might_be_in_deck();
  }

  std::optional<std::array<Card, 4>>
  issue_1634_secret_box_latias_burnet_costs() const {
    if (!issue_1634_secret_box_latias_burnet_axes_visible() ||
        hand_count(Card::SecretBox) == 0) {
      return std::nullopt;
    }

    std::map<Card, int> remaining;
    for (const Card card : state_.hand) ++remaining[card];
    --remaining[Card::SecretBox];

    std::array<Card, 4> costs{};
    std::size_t used = 0;
    const auto take = [&remaining, &costs, &used](const Card card) {
      while (used < costs.size() && remaining[card] > 0) {
        costs[used++] = card;
        --remaining[card];
      }
    };

    // The complete public route replaces these setup roles. In particular,
    // Crispin and Basic Energy cannot improve an already powered VSTAR, Steven
    // ends the current ready turn, Arven cannot improve the proven Item channel,
    // and Pineco is unnecessary once the VSTAR is already established. Preserve
    // Guzma's discrete match value until every lower-impact route-replaced card:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
    // Dynamic DCI and Secret Box route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1634
    for (const Card card : {
             Card::Grant, Card::WishfulBaton, Card::ErikasInvitation,
             Card::Crispin, Card::StevensResolve, Card::Arven,
             Card::Fire, Card::Grass, Card::Pineco, Card::Channeler,
             Card::FieldBlower, Card::ChaoticSwell, Card::PathToPeak,
             Card::TeamYellsCheer, Card::RoseannesBackup,
             Card::Lusamine, Card::ProfessorTuro, Card::Powerglass,
             Card::Guzma}) {
      take(card);
      if (used == costs.size()) return costs;
    }
    return std::nullopt;
  }

  bool play_secret_box() {
    const auto costs = issue_1634_secret_box_latias_burnet_costs();
    if (!costs) return play_secret_box_issue1634_original();

    ++outcome_.secret_box_combo_attempted;
    remove_one(state_.hand, Card::SecretBox);
    for (std::size_t index = 0; index < 3; ++index) {
      if (!discard_from_hand((*costs)[index], "Secret Box cost",
                             "R-SECRET-BOX-01; P-DCI-01")) {
        throw std::logic_error("Issue-1634 Secret Box cost plan diverged");
      }
    }
    state_.discard.push_back(Card::SecretBox);
    outcome_.used_secret_box = true;
    record_deck_search_knowledge("Secret Box issue-1634 route");

    std::vector<Card> selected;
    if (move_deck_to_hand(Card::MysteriousTreasure)) {
      selected.push_back(Card::MysteriousTreasure);
    }
    if (move_deck_to_hand(Card::ProfessorBurnet)) {
      selected.push_back(Card::ProfessorBurnet);
    }
    if (selected.size() != 2U) {
      throw std::logic_error("Issue-1634 Secret Box targets disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-SECRET-BOX-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01",
          "Secret Box discarded three route-replaced cards and searched " +
              join_cards(selected) +
              "; optional Tool and Stadium searches were left unused.");

    if (!remove_one(state_.hand, Card::MysteriousTreasure) ||
        !discard_from_hand((*costs)[3],
                           "Mysterious Treasure cost for Latias-Burnet route",
                           "R-MT-01; P-DCI-01")) {
      throw std::logic_error("Issue-1634 Mysterious Treasure cost diverged");
    }
    state_.discard.push_back(Card::MysteriousTreasure);
    record_deck_search_knowledge("Mysterious Treasure issue-1634 route");
    if (!move_deck_to_hand(Card::LatiasEx)) {
      throw std::logic_error("Issue-1634 Latias ex target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01",
          "Mysterious Treasure discarded the reserved fourth cost and searched Latias ex.");

    if (!bench_from_hand(Card::LatiasEx, true)) {
      throw std::logic_error("Issue-1634 Latias ex Bench route diverged");
    }
    if (!play_professor_burnet()) {
      throw std::logic_error("Issue-1634 Professor Burnet route diverged");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("Issue-1634 Skyliner retreat route diverged");
    }
    trace("ROUTE",
          "R-SECRET-BOX-01; R-MT-01; R-LATIAS-01; R-BURNET-01; R-RVS-01; P-COMPRESS-01",
          "Completed the K1 Secret Box, Mysterious Treasure, Latias ex, Professor Burnet, and Skyliner route.");
    return true;
  }

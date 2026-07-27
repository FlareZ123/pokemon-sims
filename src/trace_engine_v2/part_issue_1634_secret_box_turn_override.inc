  void run_secret_box_turn() {
    const bool issue_1634_axes =
        issue_1634_secret_box_latias_burnet_axes_visible();
    const bool issue_1634_costs =
        issue_1634_secret_box_latias_burnet_costs().has_value();
    const bool issue_1634_route = issue_1634_axes && issue_1634_costs;
    if (!issue_1634_route) {
      const bool powered_benched_vstar = std::any_of(
          state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
            return pokemon.card == Card::RegidragoVstar &&
                pokemon.grass >= 2 && pokemon.fire >= 1;
          });
      if (state_.turn == 4 && hand_count(Card::SecretBox) > 0 && state_.active &&
          state_.active->card == Card::DialgaGX && powered_benched_vstar) {
        int registered_latias = 0;
        for (const auto& [card, copies] : recipe_) {
          if (card == Card::LatiasEx) registered_latias += copies;
        }
        trace("POLICY", "P-KNOWLEDGE-01; P-DCI-01; R-SECRET-BOX-01",
              "Issue-1634 route gates: axes=" +
                  std::to_string(issue_1634_axes) +
                  ", costs=" + std::to_string(issue_1634_costs) +
                  ", combo=" + std::to_string(secret_box_combo_enabled()) +
                  ", K1=" + std::to_string(deck_seen_) +
                  ", item=" + std::to_string(!item_locked()) +
                  ", supporter=" + std::to_string(supporter_allowed()) +
                  ", retreat=" + std::to_string(!state_.retreat_used) +
                  ", basic=" + std::to_string(is_basic(state_.active->card)) +
                  ", bench=" + std::to_string(bench_space() > 0) +
                  ", active-axis=" + std::to_string(need_active_vstar()) +
                  ", energy=" + std::to_string(!need_energy()) +
                  ", payload=" + std::to_string(need_payload()) +
                  ", payload-turn=" +
                  std::to_string(can_play_payload_this_turn()) +
                  ", latias-ability=" +
                  std::to_string(ability_available_for_pokemon(Card::LatiasEx)) +
                  ", treasure=" + std::to_string(
                      count_of(state_.deck, Card::MysteriousTreasure) > 0) +
                  ", latias=" + std::to_string(
                      count_of(state_.deck, Card::LatiasEx) > 0) +
                  ", registered-latias=" + std::to_string(registered_latias) +
                  ", burnet=" + std::to_string(
                      count_of(state_.deck, Card::ProfessorBurnet) > 0) +
                  ", deck-payload=" +
                  std::to_string(payload_might_be_in_deck()) + ".");
      }
      run_secret_box_turn_issue1634_original();
      return;
    }

    // The registered Pineco policy otherwise reaches its Steven's Resolve branch
    // before the generic Item loop. Compare this complete K1 route at the turn
    // selector so Secret Box can use its Item and Supporter channels, Treasure
    // can find Latias ex, Burnet can establish the payload, and Skyliner can
    // promote the already powered Regidrago VSTAR during the current turn:
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, Bench, Ability, retreat, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and Secret Box policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1634
    trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
          "Start Pineco policy: " + state_line());
    if (!play_secret_box()) {
      throw std::logic_error("Issue-1634 turn-level Secret Box route diverged");
    }
    trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01; P-COMPRESS-01",
          "End Pineco policy after the Secret Box-Latias-Burnet completion: " +
              state_line());
  }

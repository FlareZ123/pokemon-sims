  void run_secret_box_turn() {
    const bool issue_1634_route =
        issue_1634_secret_box_latias_burnet_axes_visible() &&
        issue_1634_secret_box_latias_burnet_costs().has_value();
    if (!issue_1634_route) {
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

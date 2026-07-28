  bool complete_issue_1673_secret_box_payload_deadline() {
    if (!secret_box_combo_enabled() || item_locked() || state_.turn_ended ||
        state_.turn != scenario_.max_turn || !strict_payload_timing() ||
        !active_is_vstar() || need_energy() || !need_payload() ||
        hand_count(Card::SecretBox) == 0) {
      return false;
    }

    auto costs = secret_box_cost_plan();
    if (!costs ||
        !std::any_of(costs->begin(), costs->end(), is_payload)) {
      const auto payload = std::find_if(
          state_.hand.begin(), state_.hand.end(), is_payload);
      const Card surplus_energy =
          hand_count(Card::Fire) > 0 ? Card::Fire : Card::Grass;
      if (payload == state_.hand.end() ||
          hand_count(Card::WishfulBaton) == 0 ||
          hand_count(surplus_energy) == 0) {
        return false;
      }
      costs = std::array<Card, 3>{
          Card::WishfulBaton, *payload, surplus_energy};
    }

    // At the T5 deadline, the Active Regidrago VSTAR already pays GGF and the sole
    // missing axis is a Dragon entering discard this turn. Secret Box's mandatory
    // three-card cost is therefore the complete route. Wishful Baton has no modeled
    // setup value, and a held Basic Energy is surplus after GGF is established, so
    // their deadline DCI rises with the held Dragon payload. Secret Box's category
    // searches may legally fail in the hidden deck when no target improves the
    // already-complete board, while the deck is still inspected and shuffled:
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Wishful Baton: https://api.pokemontcg.io/v2/cards/sm3-128
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Hidden-zone search and fail-to-find rulings: https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    // Core Item, discard, search, shuffle, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI, AMR, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1673
    remove_one(state_.hand, Card::SecretBox);
    for (const Card cost : *costs) {
      if (!discard_from_hand(cost, "Secret Box cost",
                             "R-SECRET-BOX-01; P-DCI-01; P-JIT-01")) {
        throw std::logic_error(
            "Issue-1673 Secret Box deadline cost plan diverged from hand");
      }
    }
    state_.discard.push_back(Card::SecretBox);
    outcome_.used_secret_box = true;
    record_deck_search_knowledge("Secret Box");
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-SECRET-BOX-01; R-GAME-ITEM; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Secret Box discarded the held Dragon payload and two other cards; no searched category was needed for the completed T5 route.");
    return !need_payload();
  }

  void run_turn() {
    run_turn_issue1673_original();
    if (complete_issue_1673_secret_box_payload_deadline()) {
      trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
            "Completed the deadline Secret Box payload route: " + state_line());
    }
  }

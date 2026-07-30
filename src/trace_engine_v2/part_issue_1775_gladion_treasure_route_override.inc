#include "part_issue_1877_treasure_quick_ball_payload_bridge.inc"

  bool issue_1775_gladion_treasure_route_available() const {
    if (!prizes_known() || !strict_payload_timing() || item_locked() ||
        !supporter_allowed() || scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        state_.turn_ended || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 1 ||
        !need_energy() || !need_vstar() || !need_payload() ||
        hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::Gladion) == 0 ||
        hand_count(Card::Dragapult) == 0 ||
        prize_count_after_reveal(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0) {
      return false;
    }

    // K1 exposes one complete current-turn route. Earthen Vessel can spend the
    // held Steven's Resolve because that Supporter has no remaining setup role once
    // Vessel supplies Grass and Gladion recovers the known-prized Treasure. Treasure
    // then discards Dragapult ex as the strict-JIT payload and searches the VSTAR:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Prize exchange, attachment, Supporter, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, route-replacement DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1775
    return true;
  }

  bool complete_issue_1775_gladion_treasure_route() {
    if (!issue_1775_gladion_treasure_route_available()) return false;

    // Vessel's mandatory cost is the route-replaced Steven. It searches the one
    // missing Grass Energy, and the unused manual attachment completes GGF:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1775
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(Card::StevensResolve, "Earthen Vessel route-replacement cost",
                           "R-EV-01; P-DCI-01; P-KNOWLEDGE-01")) {
      throw std::logic_error("Issue-1775 Steven Vessel cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1775 route");
    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-1775 Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Discarded route-replaced Steven's Resolve and searched Grass Energy for the known Treasure route.");
    if (!attach_manual()) {
      throw std::logic_error("Issue-1775 Grass attachment disappeared");
    }

    // Gladion may exchange itself for the legally known prized Treasure. The
    // Supporter action reveals no hidden future draw and directly opens the missing
    // VSTAR connector:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // https://github.com/FlareZ123/pokemon-sims/issues/1775
    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto treasure_prize = std::find(
        state_.prizes.begin(), state_.prizes.end(), Card::MysteriousTreasure);
    if (treasure_prize == state_.prizes.end()) {
      throw std::logic_error("Issue-1775 known prized Treasure disappeared");
    }
    *treasure_prize = Card::Gladion;
    state_.hand.push_back(Card::MysteriousTreasure);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER",
          "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Exchanged Gladion for the known prized Mysterious Treasure that completes the T3 VSTAR route.");

    // Treasure spends Dragapult ex during the ready turn and searches Regidrago
    // VSTAR. The prior-turn Active Regidrago V may then evolve immediately:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/1775
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::Dragapult, "Mysterious Treasure strict-JIT cost",
                           "R-MT-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1775 Dragapult payload disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1775 route");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("Issue-1775 VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-MT-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Mysterious Treasure discarded Dragapult ex as the current-turn payload and searched Regidrago VSTAR.");
    evolve_best_regi();
    if (!active_is_vstar()) {
      throw std::logic_error("Issue-1775 Active VSTAR evolution disappeared");
    }
    return !need_energy() && !need_vstar() && !need_payload() &&
        !need_active_vstar();
  }

  void run_turn() {
    if (complete_issue_1877_treasure_quick_ball_payload_bridge()) {
      trace("POLICY",
            "P-AXIS-01; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; R-MT-01; R-QB-01",
            "Completed the K1 Treasure-to-Quick-Ball payload bridge: " + state_line());
      return;
    }
    if (complete_issue_1775_gladion_treasure_route()) {
      trace("POLICY",
            "P-AXIS-01; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; R-GLADION-01; R-EV-01; R-MT-01",
            "Completed the K1 Gladion-Treasure T3 route: " + state_line());
      return;
    }
    run_turn_issue1775_original();
  }

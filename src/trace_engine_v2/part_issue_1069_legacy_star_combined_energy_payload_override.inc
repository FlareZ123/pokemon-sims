  bool held_payload_only_outlet_blocks_combined_legacy_star_route() const {
    // These held cards can establish a current-turn Dragon payload, but none can
    // supply the missing Basic Energy by itself. They therefore cannot suppress a
    // Legacy Star line that is needed to bridge both axes:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Confirmed non-Blender regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
    const bool held_item_payload_only_outlet = !item_locked() &&
        (hand_count(Card::BrilliantBlender) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0);
    const bool held_supporter_payload_only_outlet =
        supporter_allowed() && hand_count(Card::ProfessorBurnet) > 0;
    if ((!held_item_payload_only_outlet && !held_supporter_payload_only_outlet) ||
        !active_is_vstar() || state_.manual_energy_used || !need_energy() ||
        !need_payload() || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck() || state_.deck.empty()) {
      return false;
    }

    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    if (missing_grass + missing_fire != 1) return false;

    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    return hand_count(needed_energy) == 0 && might_be_unseen(needed_energy);
  }

  bool use_legacy_star() {
    const bool issue_1817_hold_for_burnet_finish =
        issue_1817_burnet_finish_pending_ &&
        scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::TurnTwoItem && state_.turn == 2 &&
        scenario_.max_turn >= 3 && deck_seen_ && state_.manual_energy_used &&
        active_is_vstar() && state_.active->grass == 1 &&
        state_.active->fire == 1 && hand_count(Card::Grass) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 && need_payload();
    if (issue_1817_hold_for_burnet_finish) {
      // VSTAR, the held final Grass, and held Professor Burnet already guarantee
      // T3 readiness. Preserve the one-use VSTAR Power instead of introducing a
      // random discard and recovery sequence that cannot improve the deadline:
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official VSTAR Power and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Resource-preservation and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1817
      return false;
    }

    const bool issue_1765_hold_for_t3_payload_window =
        scenario_.dci == DciProfile::StrictJit && scenario_.locks == LockMode::None &&
        !scenario_.going_first && state_.turn == 2 && scenario_.max_turn >= 3 &&
        deck_seen_ && state_.manual_energy_used && active_is_vstar() &&
        grass_needed() == 0 && fire_needed() == 1 && need_payload() &&
        hand_count(Card::Fire) > 0 && hand_count(Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0;
    if (issue_1765_hold_for_t3_payload_window) {
      // The surplus-Grass Treasure route completes the VSTAR and Grass axes on T2,
      // while the held Fire completes GGF on T3. Strict JIT cannot credit a T2
      // Legacy Star payload on T3, so preserve the one-use VSTAR Power for the first
      // turn that can complete every setup axis:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // VSTAR Power and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1765
      return false;
    }

    if (!held_payload_only_outlet_blocks_combined_legacy_star_route()) {
      return use_legacy_star_issue1069_original();
    }

    const int held_blenders = hand_count(Card::BrilliantBlender);
    const int held_treasures = hand_count(Card::MysteriousTreasure);
    const int held_burnets = hand_count(Card::ProfessorBurnet);
    std::erase(state_.hand, Card::BrilliantBlender);
    std::erase(state_.hand, Card::MysteriousTreasure);
    std::erase(state_.hand, Card::ProfessorBurnet);
    const bool used = use_legacy_star_issue1069_original();
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_blenders),
                       Card::BrilliantBlender);
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_treasures),
                       Card::MysteriousTreasure);
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_burnets),
                       Card::ProfessorBurnet);

    // Legacy Star may recover any two cards after its seven-card discard. When a
    // payload-only outlet is the sole preflight suppressor, temporarily excluding
    // that outlet lets the existing adaptive recovery policy evaluate the missing
    // Energy axis as well. The held cards are restored immediately after the VSTAR
    // Power evaluation, so later same-turn play can still use them when required:
    // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Official Item, Supporter, Ability, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Original Blender regression: https://github.com/FlareZ123/pokemon-sims/issues/1069
    // Confirmed non-Blender regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
    return used;
  }

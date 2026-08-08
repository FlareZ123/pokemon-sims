  bool issue_2165_burnet_treasure_vstar_route_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        item_locked() || state_.supporter_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::UltraBall) == 0 ||
        hand_count(Card::Dragapult) == 0 ||
        hand_count(Card::RegidragoVstar) > 0) {
      return false;
    }
    const bool tapu_benched = std::any_of(
        state_.bench.begin(), state_.bench.end(),
        [](const Pokemon& pokemon) { return pokemon.card == Card::TapuLeleGX; });
    if (!tapu_benched) return false;

    // Mysterious Treasure pays one discard and searches a Dragon or Psychic
    // Pokemon. In this public singleton-heavy hand, Ultra Ball is the redundant
    // lower-AMR connector because it requires two later discards, while Treasure
    // immediately establishes the missing Regidrago VSTAR axis and K1 before the
    // one-per-turn Supporter decision. Professor Burnet remains available for a
    // later payload decision rather than preempting the advancing Item route:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Trainer, search, discard-cost, and one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1 and earliest public route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI and AMR assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2165
    return issue_1419_publicly_possible_count(Card::RegidragoVstar) > 0;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (issue_2265_hold_treasure_for_direct_route()) {
      // The scheduled Fire attachment, held VSTAR, K1-known Quick Ball and Latias,
      // held Arven, and held Brilliant Blender already prove a complete same-turn
      // route. Preserve Treasure and its discard cost so the direct Arven line can
      // resolve after attachment instead of spending an extra connector and Bench
      // slot on Tapu Lele-GX / Wonder Tag.
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Core attachment, evolution, Supporter, Item, Bench, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
      return false;
    }
    if (!issue_2165_burnet_treasure_vstar_route_available()) {
      return play_mysterious_treasure_issue2165_original(permit_payload);
    }

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::UltraBall, "Mysterious Treasure cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-2165 Ultra Ball cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");

    if (move_deck_to_hand(Card::RegidragoVstar)) {
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
            "Discarded redundant Ultra Ball and searched Regidrago VSTAR before the Turn 1 Supporter decision.");
      return true;
    }

    const Card fallback = fallback_mysterious_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          found_fallback
              ? "Discarded redundant Ultra Ball; Regidrago VSTAR was unavailable after K1 inspection, so Mysterious Treasure searched " +
                    std::string(name(fallback)) + "."
              : "Discarded redundant Ultra Ball; no Dragon or Psychic Pokemon remained after K1 inspection.");
    return true;
  }

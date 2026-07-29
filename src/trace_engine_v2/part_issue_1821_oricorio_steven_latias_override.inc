  bool issue_1821_public_route_candidate() const {
    const bool active_oricorio = state_.active &&
        state_.active->card == Card::Oricorio &&
        state_.active->grass == 0 && state_.active->fire == 0;
    const bool benched_tapu = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::TapuLeleGX;
        });
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);

    // The exact public K1 route spends the held Grass Energy on Oricorio's printed
    // one-Energy Retreat Cost, promotes the already Benched Tapu Lele-GX, and uses
    // Steven's Resolve to bank the missing Basic, Latias ex, and the first T2 manual
    // Grass attachment. Held Regidrago VSTAR, Earthen Vessel, and a Dragon payload
    // already complete the evolution-card and strict-JIT outlet axes:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, Supporter, search, Bench, evolution, Item, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, connector compression, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1821
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && !scenario_.going_first &&
        state_.turn == 1 && scenario_.max_turn >= 3 && prizes_known() &&
        supporter_allowed() && !state_.manual_energy_used &&
        !state_.retreat_used && active_oricorio && benched_tapu &&
        bench_space() >= 2 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::RegidragoV) == 0 &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::Grass) > 0 && held_payload;
  }

  bool issue_1821_targets_available() const {
    if (!deck_seen_ || !issue_1821_public_route_candidate()) return false;

    // Steven searches one Grass now. Preserve enough K1 Basic Energy inventory for
    // the mandatory T2 draw to remove any one Grass or Fire while Crispin still finds
    // two different types. This avoids future-card oracle dependence:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Official draw and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1821
    return deck_count_after_search_started(Card::RegidragoV) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 3 &&
        deck_count_after_search_started(Card::Fire) >= 2;
  }

  bool play_issue_1821_oricorio_steven_route() {
    if (!issue_1821_targets_available()) return false;

    auto tapu = std::find_if(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::TapuLeleGX;
        });
    if (tapu == state_.bench.end()) return false;

    // Attach one Basic Energy, then discard that attached Energy to pay Oricorio's
    // printed one-Colorless Retreat Cost before the turn-ending Supporter:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    if (!remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue 1821 retreat Energy disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-ORICORIO-01; P-COMPRESS-01",
          "Grass Energy manually to Oricorio for the deterministic Steven route.");

    --state_.active->grass;
    state_.discard.push_back(Card::Grass);
    std::swap(*state_.active, *tapu);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT; R-ORICORIO-01; R-TAPU-01",
          "Paid Oricorio's one-Energy Retreat Cost and promoted Tapu Lele-GX before Steven's Resolve ended the turn.");

    // Steven searches any three cards, then ends the turn. The selected package
    // preserves every already-held axis and supplies the missing Basic, promotion,
    // and attachment channels:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://github.com/FlareZ123/pokemon-sims/issues/1821
    if (!remove_one(state_.hand, Card::StevensResolve)) {
      throw std::logic_error("Issue 1821 Steven's Resolve disappeared");
    }
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1821 route");

    std::vector<Card> selected;
    for (const Card card : {Card::RegidragoV, Card::LatiasEx, Card::Grass}) {
      if (move_deck_to_hand(card)) selected.push_back(card);
    }
    if (selected.size() != 3U) {
      throw std::logic_error("Issue 1821 Steven targets disappeared after K1 validation");
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Steven's Resolve searched Regidrago V, Latias ex, and Grass Energy for the deterministic T3 route; the held VSTAR and Vessel-payload channel were preserved and the turn ended.");
    return true;
  }

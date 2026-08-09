  bool issue_2622_one_held_attachment_completes_apex() const {
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV) continue;

      for (const Card energy : {Card::Grass, Card::Fire, Card::DoubleDragonEnergy}) {
        if (hand_count(energy) == 0) continue;
        Pokemon projected = pokemon;
        if (attach_energy_card(projected, energy) && pays_apex_energy_cost(projected)) {
          return true;
        }
      }
    }
    return false;
  }

  bool issue_2622_steven_latias_blender_package_available() const {
    if (!deck_seen_ || !supporter_allowed() ||
        hand_count(Card::StevensResolve) == 0 ||
        scenario_.dci != DciProfile::StrictJit ||
        state_.turn >= scenario_.max_turn || !state_.active ||
        !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar ||
        !need_vstar() || !need_energy() || !need_payload() ||
        bench_space() == 0 || in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        held_arven_fss_blender_finish_available() ||
        !issue_2622_one_held_attachment_completes_apex()) {
      return false;
    }

    const bool next_turn_item_locked =
        scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        (scenario_.locks == LockMode::TurnTwoItem && state_.turn == 1);
    if (next_turn_item_locked) return false;

    const auto available_after_steven = [this](const Card card) {
      return hand_count(card) > 0 || deck_count_after_search_started(card) > 0;
    };
    if (!available_after_steven(Card::RegidragoVstar) ||
        !available_after_steven(Card::LatiasEx) ||
        !available_after_steven(Card::BrilliantBlender)) {
      return false;
    }

    const int targets_needed =
        static_cast<int>(hand_count(Card::RegidragoVstar) == 0) +
        static_cast<int>(hand_count(Card::LatiasEx) == 0) +
        static_cast<int>(hand_count(Card::BrilliantBlender) == 0);
    if (targets_needed == 0 || targets_needed > 3) return false;

    const int payloads_remaining = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_payload));

    // Steven's Resolve can search up to three unrestricted cards and then ends the
    // turn. This route is admitted only after K1 proves that those search slots can
    // reserve every missing VSTAR/Latias/Blender piece, a held Energy attachment
    // completes Apex next turn, and at least two Dragon payloads remain in deck so
    // the mandatory next-turn draw cannot strand Brilliant Blender. Latias ex then
    // gives the Basic Active no Retreat Cost into the evolved Regidrago VSTAR.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Basic Energy attachment, evolution, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2622
    return payloads_remaining >= 2;
  }

  bool play_issue_2622_steven_latias_blender_package() {
    if (!issue_2622_steven_latias_blender_package_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-2622 package");

    std::vector<Card> found;
    const auto find_if_missing = [this, &found](const Card card) {
      if (hand_count(card) > 0) return;
      if (!move_deck_to_hand(card)) {
        throw std::logic_error("Issue-2622 Steven package target disappeared");
      }
      found.push_back(card);
    };
    find_if_missing(Card::RegidragoVstar);
    find_if_missing(Card::LatiasEx);
    find_if_missing(Card::BrilliantBlender);
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Search exactly the missing deterministic package and preserve the Supporter
    // choice over Arven/FSS when Arven cannot also solve the Basic-Active mobility
    // axis this turn. Steven says "up to 3 cards", so fewer targets remain legal
    // when one of the package pieces is already held.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official turn, search, evolution, attachment, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest deterministic route specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2622
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the deterministic next-turn VSTAR-Latias-Blender package: " +
              join_cards(found) + ". Turn ended.");
    return true;
  }

  bool play_steven() {
    if (play_issue_2622_steven_latias_blender_package()) return true;
    return play_steven_issue2622_original();
  }

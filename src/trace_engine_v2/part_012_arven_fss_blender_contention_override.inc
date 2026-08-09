  bool arven_final_energy_vessel_dead_role_route_live() const {
    if (!deck_seen_ || !supporter_allowed() || hand_count(Card::Arven) == 0 ||
        item_locked() || scenario_.dci != DciProfile::NoDiscardControl ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_payload() ||
        deck_count_after_search_started(Card::EarthenVessel) == 0) {
      return false;
    }

    const auto completes_apex = [this](const Card basic) {
      if (deck_count_after_search_started(basic) == 0) return false;
      Pokemon projected = *state_.active;
      return attach_energy_card(projected, basic) && pays_apex_energy_cost(projected);
    };
    const bool needed_energy_searchable =
        completes_apex(Card::Grass) || completes_apex(Card::Fire);
    const bool dead_role_held = hand_count(Card::QuickBall) > 0 ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::LatiasEx) > 0 || hand_count(Card::Gladion) > 0;

    // Project the physical final Basic attachment against the actual attacker.
    // With DDE attached, Grass and Fire are alternative one-card Apex finishes:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Repository DDE contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2426
    // Active Regidrago VSTAR and payload are already complete. Arven can search
    // Earthen Vessel, one now-dead setup role can legally pay its printed discard
    // cost, and the searched Basic can use the unused manual attachment:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Manual attachment: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1050
    return needed_energy_searchable && dead_role_held;
  }

  bool arven_fss_blender_contention_route_live() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2076

    if (!prizes_known() || !supporter_allowed() || hand_count(Card::Arven) == 0 ||
        item_locked() || state_.vstar_power_used || !can_play_payload_this_turn()) {
      return false;
    }

    const auto powered_evolvable_regi = [this](const Pokemon& pokemon) {
      // DDE + Grass or DDE + Fire already pays Apex Dragon on Regidrago V, and
      // evolution preserves the attached Energy on the resulting VSTAR:
      // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Evolution keeps attached cards: https://www.pokemon.com/us/pokemon-tcg/rules
      // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2426
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn &&
             pays_apex_energy_cost(pokemon) && pokemon.tool == Tool::None;
    };
    const bool active_route = state_.active && powered_evolvable_regi(*state_.active);
    const bool benched_route = can_free_retreat_with_latias() &&
        std::any_of(state_.bench.begin(), state_.bench.end(), powered_evolvable_regi);
    if (!active_route && !benched_route) return false;

    const auto in_deck = [this](const Card card) {
      return std::find(state_.deck.begin(), state_.deck.end(), card) != state_.deck.end();
    };
    const bool payload_in_deck = std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    const bool vstar_already_held = hand_count(Card::RegidragoVstar) > 0;
    // A held Regidrago VSTAR already solves the card axis, so Arven needs only
    // Brilliant Blender for the current-turn payload. Forest Seal Stone remains
    // necessary only when Star Alchemy must search the missing VSTAR:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1346
    return in_deck(Card::BrilliantBlender) && payload_in_deck &&
           (vstar_already_held ||
            (in_deck(Card::ForestSealStone) && in_deck(Card::RegidragoVstar)));
  }

  bool arven_appletun_payload_route_live() const {
    if (!supporter_allowed() || hand_count(Card::Arven) == 0 ||
        !can_play_payload_this_turn() || !need_payload() || need_regi() ||
        need_vstar() || need_energy() || need_active_vstar() ||
        !might_be_unseen(Card::Appletun)) {
      return false;
    }

    // Appletun is a Stage 1 Dragon and a modeled Apex Dragon payload. Arven may
    // take Brilliant Blender for a current-turn discard, or take Forest Seal Stone
    // so Star Alchemy can bank Professor Burnet for the next modeled turn:
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR and Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tool, Supporter, search, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1293
    const bool direct_blender_route =
        !item_locked() && might_be_unseen(Card::BrilliantBlender);
    const bool open_v_tool_slot =
        (state_.active && is_pokemon_v(state_.active->card) &&
         state_.active->tool == Tool::None) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
        });
    const bool next_turn_burnet_route = state_.turn < scenario_.max_turn &&
        !state_.vstar_power_used && open_v_tool_slot &&
        might_be_unseen(Card::ForestSealStone) &&
        might_be_unseen(Card::ProfessorBurnet);
    return direct_blender_route || next_turn_burnet_route;
  }

bool play_issue_1605_arven_redundant_payload_route() {
  if (!issue_1605_arven_crobat_route_available()) return false;

  // This decision is made from the public K0 hand, board, lock state, and fixed
  // deck recipe. Arven then performs the real legal deck inspection. Prefer a
  // physical Quick Ball, then Mysterious Treasure, and voluntarily fail the Tool
  // search because Forest Seal Stone is already held. The route flag is enabled
  // only when the inspected deck actually supplies the one-discard Item:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Core Supporter, Item, discard, search, Bench, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Typed searches may voluntarily fail: https://compendium.pokegym.net/compendium-bw.html#trainers-in-general
  // K0/K1, dynamic DCI, UDP, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
  remove_one(state_.hand, Card::Arven);
  state_.discard.push_back(Card::Arven);
  state_.supporter_used = true;
  record_deck_search_knowledge("Arven issue-1605 route");

  std::optional<Card> found_item;
  if (deck_count_after_search_started(Card::QuickBall) > 0 &&
      move_deck_to_hand(Card::QuickBall)) {
    found_item = Card::QuickBall;
  } else if (deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
             move_deck_to_hand(Card::MysteriousTreasure)) {
    found_item = Card::MysteriousTreasure;
  }
  issue_1605_arven_redundant_payload_route_ = found_item.has_value();
  shuffle(state_.deck);
  trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01; P-CONNECTOR-01",
        found_item
            ? "Arven searched " + std::string(name(*found_item)) +
                  " and preserved the held Forest Seal Stone for the Crobat continuation."
            : "Arven legally inspected the deck, but no one-discard Regidrago search Item remained.");
  return true;
}

bool play_arven() {
  if (play_issue_1605_arven_redundant_payload_route()) return true;
    if (arven_appletun_payload_route_live()) {
      remove_one(state_.hand, Card::Arven);
      state_.discard.push_back(Card::Arven);
      state_.supporter_used = true;
      record_deck_search_knowledge("Arven");

      std::vector<Card> found;
      const bool appletun_present =
          deck_count_after_search_started(Card::Appletun) > 0;
      if (appletun_present && !item_locked() &&
          deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
          move_deck_to_hand(Card::BrilliantBlender)) {
        found.push_back(Card::BrilliantBlender);
      } else if (appletun_present && state_.turn < scenario_.max_turn &&
                 deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
                 deck_count_after_search_started(Card::ForestSealStone) > 0 &&
                 move_deck_to_hand(Card::ForestSealStone)) {
        found.push_back(Card::ForestSealStone);
      }
      shuffle(state_.deck);
      trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
            "Searched the live Appletun payload connector: " + join_cards(found) + ".");
      return true;
    }

    if (arven_final_energy_vessel_dead_role_route_live()) {
      remove_one(state_.hand, Card::Arven);
      state_.discard.push_back(Card::Arven);
      state_.supporter_used = true;
      record_deck_search_knowledge("Arven");
      const bool found = move_deck_to_hand(Card::EarthenVessel);
      shuffle(state_.deck);
      trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01",
            found ? "Searched Earthen Vessel for the final Basic Energy route."
                  : "Earthen Vessel was absent after legal deck inspection.");
      return true;
    }

    if (!arven_fss_blender_contention_route_live()) {
      return play_arven_fss_blender_contention_original();
    }

    // Arven's Item and Tool searches are separate resources. When a powered,
    // evolvable Regidrago V already has a legal Active route, Forest Seal Stone can
    // use Star Alchemy for Regidrago VSTAR while Brilliant Blender supplies the
    // remaining Dragon payload. Searching Mysterious Treasure here duplicates the
    // VSTAR connector and delays the earliest legal ready state:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Core evolution, Tool, Ability, Item, and retreat rules:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready and connector priorities:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/745
    // DDE refinement: https://github.com/FlareZ123/pokemon-sims/issues/2426
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven");

    const bool vstar_already_held = hand_count(Card::RegidragoVstar) > 0;
    std::vector<Card> found;
    if (move_deck_to_hand(Card::BrilliantBlender)) found.push_back(Card::BrilliantBlender);
    // Arven may leave the Tool search unresolved when the held VSTAR already
    // completes that axis. Preserve Forest Seal Stone and the open Tool slot:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // Typed deck searches may voluntarily fail: https://compendium.pokegym.net/compendium-bw.html#trainers-in-general
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1346
    if (!vstar_already_held && move_deck_to_hand(Card::ForestSealStone)) {
      found.push_back(Card::ForestSealStone);
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched an Item and a Pokemon Tool without duplicating the VSTAR connector: " +
              join_cards(found) + ".");
    return true;
  }

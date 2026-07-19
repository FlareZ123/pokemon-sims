    state_.supporter_used = true;
    outcome_.used_steven = true;

    // Card text source: https://api.pokemontcg.io/v2/cards/sm7-145
    // Steven's Resolve is a legal deck inspection. It can therefore reserve
    // Prize bridges only after the deck inspection establishes K1 knowledge.
    record_deck_search_knowledge("Steven's Resolve");
    const bool next_turn_item_locked = scenario_.locks == LockMode::FullItem ||
                                       scenario_.locks == LockMode::FullCombined ||
                                       (scenario_.locks == LockMode::TurnTwoItem && state_.turn == 1);
    const bool blender_available_next_turn = !next_turn_item_locked && payload_might_be_in_deck() &&
                                             deck_count_after_search_started(Card::BrilliantBlender) > 0;
    // Burnet directly searches and discards up to 2 cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Steven's Resolve reserves an outlet only where it can still find a payload.
    const Card payload_outlet = blender_available_next_turn ? Card::BrilliantBlender : Card::ProfessorBurnet;
    std::vector<Card> wanted;
    const auto add_wanted = [&wanted](const Card card) {
      if (std::find(wanted.begin(), wanted.end(), card) == wanted.end()) wanted.push_back(card);
    };

    if (known_t3_compression) {
      // These exact K1 targets complete the confirmed T3 route; generic fallbacks may
      // append later, but Steven resolves only the first three targets in this order:
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/me2pt5-152
      // https://github.com/FlareZ123/pokemon-sims/issues/869
      add_wanted(Card::LatiasEx);
      add_wanted(Card::MysteriousTreasure);
      add_wanted(Card::MegaDragonite);
    }

    // Steven searches up to three unrestricted cards at once. Project prerequisite
    // targets before choosing later axes: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V evolves into Regidrago VSTAR. Gladion exchanges for exactly one
    // Prize card, while Hisuian Heavy Ball can recover a prized Basic independently:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // Repository route priority and multi-Prize accounting:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed defect: https://github.com/FlareZ123/pokemon-sims/issues/867
    const bool regi_in_deck = deck_count_after_search_started(Card::RegidragoV) > 0;
    const bool vstar_in_deck = deck_count_after_search_started(Card::RegidragoVstar) > 0;
    const bool regi_known_prized = prize_count_after_reveal(Card::RegidragoV) > 0;
    const bool vstar_known_prized = prize_count_after_reveal(Card::RegidragoVstar) > 0;
    // Steven ends the turn, so a Heavy Ball bridge must remain legal on the next turn:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    const bool heavy_ball_available_next_turn = !next_turn_item_locked &&
        (hand_count(Card::HisuianHeavyBall) > 0 ||
         deck_count_after_search_started(Card::HisuianHeavyBall) > 0);
    const bool gladion_available = hand_count(Card::Gladion) > 0 ||
                                   deck_count_after_search_started(Card::Gladion) > 0;
    const auto reserve_if_needed = [&add_wanted, this](const Card card) {
      if (hand_count(card) == 0) add_wanted(card);
    };

    bool projected_regi_available = has_regi_v() || hand_count(Card::RegidragoV) > 0;
    bool projected_regi_uses_gladion = false;
    bool projected_vstar_uses_gladion = false;
    if (need_regi() && bench_space() > 0 && hand_count(Card::RegidragoV) == 0) {
      if (regi_in_deck) {
        add_wanted(Card::RegidragoV);
        projected_regi_available = true;
      } else if (regi_known_prized && gladion_available) {
        // One Gladion cannot recover both a prized Basic and a prized VSTAR. Reserve
        // Heavy Ball for the Basic only when that Item will remain legal next turn.
        if (!vstar_in_deck && vstar_known_prized && heavy_ball_available_next_turn) {
          reserve_if_needed(Card::HisuianHeavyBall);
          reserve_if_needed(Card::Gladion);
          projected_regi_available = true;
          projected_vstar_uses_gladion = true;
        } else {
          reserve_if_needed(Card::Gladion);
          projected_regi_available = true;
          projected_regi_uses_gladion = true;
        }
      }
    }

    const bool projected_vstar_missing = projected_regi_available && !has_vstar() &&
                                         hand_count(Card::RegidragoVstar) == 0;
    bool projected_vstar_direct = false;
    if (projected_vstar_missing) {
      if (vstar_in_deck) {
        add_wanted(Card::RegidragoVstar);
        projected_vstar_direct = true;
      } else if (vstar_known_prized && gladion_available && !projected_regi_uses_gladion) {
        reserve_if_needed(Card::Gladion);
        projected_vstar_uses_gladion = true;
      }
    }

    // Steven's Resolve searches up to three unrestricted cards. A Pineco already
    // in play makes Forretress ex a legal next-turn Stage 1 Energy connector. If
    // Pineco is still missing, Steven may reserve the Basic before later axes:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/972
    if (wanted.size() < 3U && needs_pineco_connector() &&
        deck_count_after_search_started(Card::Pineco) > 0) {
      add_wanted(Card::Pineco);
    }
    if (wanted.size() < 3U && needs_forretress_connector() &&
        deck_count_after_search_started(Card::ForretressEx) > 0) {
      add_wanted(Card::ForretressEx);
    }

    const bool projected_vstar_available = has_vstar() || hand_count(Card::RegidragoVstar) > 0 ||
        projected_vstar_direct || projected_vstar_uses_gladion;
    const bool held_routes_complete_next_turn = steven_held_routes_complete_next_turn(
        wanted, next_turn_item_locked, projected_vstar_available);

    // Crispin source: https://api.pokemontcg.io/v2/cards/sv7-133
    if (!held_routes_complete_next_turn && need_energy() &&
        hand_count(Card::Crispin) == 0 && crispin_can_advance_energy_axis() &&
        deck_count_after_search_started(Card::Crispin) > 0) {
      add_wanted(Card::Crispin);
    }

    const bool projected_crispin_available = hand_count(Card::Crispin) > 0 ||
        std::find(wanted.begin(), wanted.end(), Card::Crispin) != wanted.end();
    const Pokemon* projected_regi = target_regi();
    const bool projected_legacy_star_payload_route = projected_vstar_available &&
        !state_.vstar_power_used && ability_available_for_pokemon(Card::RegidragoVstar) &&
        payload_might_be_in_deck();
    const bool payload_route_preserved = !need_payload() || held_routes_complete_next_turn ||
        (!next_turn_item_locked && hand_count(Card::BrilliantBlender) > 0) ||
        hand_count(Card::ProfessorBurnet) > 0 || projected_legacy_star_payload_route;
    const bool basic_active_needs_promotion = state_.active && is_basic(state_.active->card) &&
                                              state_.active->card != Card::RegidragoV;
    const bool crispin_and_manual_complete_ggf = projected_regi != nullptr &&
        projected_regi->grass >= 1 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    // Latias ex gives Basic Pokémon no Retreat Cost. Once Steven's first targets
    // establish VSTAR plus GGF behind a Basic Active, promotion outranks a redundant
    // Pokémon fallback: https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (wanted.size() < 3U && projected_vstar_available && projected_crispin_available &&
        crispin_and_manual_complete_ggf && payload_route_preserved && basic_active_needs_promotion &&
        bench_space() > 0 && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        deck_count_after_search_started(Card::LatiasEx) > 0) {
      add_wanted(Card::LatiasEx);
    }

    if (!held_routes_complete_next_turn && wanted.size() < 3U && need_payload() &&
        payload_might_be_in_deck() && hand_count(Card::BrilliantBlender) == 0 &&
        hand_count(Card::ProfessorBurnet) == 0 &&
        deck_count_after_search_started(payload_outlet) > 0) {
      add_wanted(payload_outlet);
    }
    if (!held_routes_complete_next_turn) {
      for (const Card fallback : {Card::RegidragoV, Card::RegidragoVstar, Card::Gladion,
                                  Card::Crispin, payload_outlet, Card::MysteriousTreasure,
                                  Card::EarthenVessel, Card::LatiasEx}) {
        if (wanted.size() >= 3U) break;
        if (fallback == Card::Crispin && !crispin_can_advance_energy_axis()) continue;
        if (fallback == payload_outlet && !payload_might_be_in_deck()) continue;
        add_wanted(fallback);
      }
    }
    std::vector<Card> found;
    for (const Card target : wanted) {
      if (found.size() >= 3U) break;

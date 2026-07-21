    const auto second = choose_discard(permit_payload && can_play_payload_this_turn() && !is_payload(*first), true);
    state_.hand.push_back(*first);
    if (!second) return false;

    remove_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first, "Ultra Ball cost", "R-UB-01");
    discard_from_hand(*second, "Ultra Ball cost", "R-UB-01");
    // Ultra Ball searches for any Pokémon after its two-card cost: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    record_deck_search_knowledge("Ultra Ball");

    // A fetched Dragon can become this turn's strict-JIT payload through the separate
    // hand-discard outlet selected in play_ultra_ball: https://api.pokemontcg.io/v2/cards/swsh12-136
    const std::array<Card, 9> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                                      Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                      Card::DialgaGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (want_payload && is_payload(target));
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM", "Searched a Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    const Card fallback = fallback_ultra_ball_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Pokémon remained after search.");
    return true;
  }

  bool play_evolution_incense(const bool permit_payload = false) {
    if (item_locked() || hand_count(Card::EvolutionIncense) == 0) return false;
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0;
    // Evolution Incense can search any Evolution Pokémon. A JIT payload search is
    // live only when a separate card already in hand can discard that payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    const bool hand_payload_discard_outlet = hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        (supporter_allowed() && hand_count(Card::Serena) > 0);
    const bool want_payload = permit_payload && can_play_payload_this_turn() && need_payload() &&
                              hand_payload_discard_outlet;
    if (!want_vstar && !want_payload) return false;

    remove_one(state_.hand, Card::EvolutionIncense);
    state_.discard.push_back(Card::EvolutionIncense);
    record_deck_search_knowledge("Evolution Incense");
    bool found = false;
    Card found_card = Card::RegidragoVstar;
    if (want_vstar && move_deck_to_hand(Card::RegidragoVstar)) {
      found = true;
      found_card = Card::RegidragoVstar;
    }
    // The modeled Evolution payloads are legal Evolution Incense targets:
    // Mega Dragonite ex https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Dragapult ex https://api.pokemontcg.io/v2/cards/sv6-130
    // Hisuian Goodra VSTAR https://api.pokemontcg.io/v2/cards/swsh11-136
    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar}) {
        if (move_deck_to_hand(payload)) {
          found = true;
          found_card = payload;
          break;
        }
      }
    }
    if (!found && move_deck_to_hand(Card::GoodraVstar)) { found = true; found_card = Card::GoodraVstar; }
    if (!found && move_deck_to_hand(Card::Dipplin)) { found = true; found_card = Card::Dipplin; }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EVOINCENSE-01; R-GAME-ITEM", found ? "Searched an Evolution Pokémon: " + std::string(name(found_card)) + "." : "No Evolution Pokémon remained in deck.");
    return true;
  }

  bool hold_payload_outlet_for_post_powerglass_turn() const {
    if (!strict_payload_timing() || item_locked() || !need_payload() || !need_energy() ||
        state_.turn >= scenario_.max_turn || !state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || state_.active->tool != Tool::None ||
        hand_count(Card::Powerglass) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    // Powerglass resolves after the attack step. Preserve a one-discard payload
    // outlet for the next turn only when its Active holder lacks exactly one Energy
    // and that Energy is already in discard:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    return active_grass_needed + active_fire_needed == 1 &&
        ((active_grass_needed == 1 && count_of(state_.discard, Card::Grass) > 0) ||
         (active_fire_needed == 1 && count_of(state_.discard, Card::Fire) > 0));
  }

  bool play_earthen_vessel(const bool permit_payload) {
    if (item_locked() || hand_count(Card::EarthenVessel) == 0) return false;
    // Do not discard the held Dragon before Powerglass completes GGF after the attack
    // step. The recovered Vessel remains live on the first post-Powerglass turn:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    if (permit_payload && hold_payload_outlet_for_post_powerglass_turn()) return false;
    // Once a different current-turn Dragon already satisfies strict-JIT, another
    // modeled Dragon remains a legal Earthen Vessel cost and adds an Apex Dragon option:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    const bool can_jit_discard = permit_payload &&
        (can_play_payload_this_turn() || payload_ready()) &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Earthen Vessel searches Basic Energy only after discarding another card. Once
    // K1 proves no needed modeled Basic Energy remains, preserve both hand cards
    // unless Vessel is still the current-turn outlet for an in-hand Dragon payload:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool needed_energy_might_be_in_deck =
        (grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
        (fire_needed() > 0 && might_be_unseen(Card::Fire));
    if ((!need_energy() || !needed_energy_might_be_in_deck) && !can_jit_discard) return false;
    // The Earthen Vessel being played cannot discard itself. A held Ultra Ball is
    // a distinct card in hand and remains a legal cost even when Ultra Ball cannot
    // pay its own two-card play cost: https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const auto cost = choose_discard(can_jit_discard, true, true, Card::EarthenVessel);
    if (!cost) return false;
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(*cost, "Earthen Vessel cost", "R-EV-01");
    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM", "Earthen Vessel");
    return true;
  }

  bool play_brilliant_blender() {
    // Blender can only discard cards found in deck: https://api.pokemontcg.io/v2/cards/sv8-164
    // After K1 proves every payload is absent from deck, preserve Blender for a
    // later shuffle instead of consuming the Item on a dead payload route.
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck()) return false;
    remove_one(state_.hand, Card::BrilliantBlender);
    state_.discard.push_back(Card::BrilliantBlender);
    std::vector<Card> discarded;
    for (const Card payload : {Card::Appletun, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_deck_to_discard(payload)) { discarded.push_back(payload); break; }
    }
    if (move_deck_to_discard(Card::Dipplin)) discarded.push_back(Card::Dipplin);
    shuffle(state_.deck);
    outcome_.used_blender = true;
    trace("PLAY ITEM", "R-BLENDER-01; R-GAME-ITEM; P-DCI-01", "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

  bool attach_fss() {
    // Pokémon Tool cards are a category separate from Item cards, so Item lock does not
    // prohibit attaching Forest Seal Stone: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    // Forest Seal Stone may be attached only to a Pokémon V: https://api.pokemontcg.io/v2/cards/swsh12-156
    if (hand_count(Card::ForestSealStone) == 0) return false;
    Pokemon* target = nullptr;
    if (state_.active && is_pokemon_v(state_.active->card) && state_.active->tool == Tool::None) target = &*state_.active;
    if (target == nullptr) {
      for (Pokemon& p : state_.bench) {
        if (is_pokemon_v(p.card) && p.tool == Tool::None) { target = &p; break; }
      }
    }
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::ForestSealStone);
    target->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL", "Attached Forest Seal Stone to Regidrago V.");
    return true;
  }

  bool should_play_steven() const {
    if (state_.turn != 1 || scenario_.going_first) return false;
    // Steven's Resolve can establish a missing Basic Regidrago V before the turn ends;
    // Regidrago V is a Basic Pokémon: https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/swsh12-135
    const bool missing_regi_axis = need_regi() && bench_space() > 0 && hand_count(Card::RegidragoV) == 0;
    const int axes = static_cast<int>(missing_regi_axis) +
                     static_cast<int>(need_vstar() && hand_count(Card::RegidragoVstar) == 0) +
                     static_cast<int>(need_energy() && hand_count(Card::Crispin) == 0) +
                     static_cast<int>(need_payload() && payload_might_be_in_deck() &&
                                      hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0);
    return axes >= 2;
  }

  bool can_fetch_oricorio_for_compression() const {
    return need_energy() && bench_space() > 0 && !in_play(Card::Oricorio) &&
           hand_count(Card::Oricorio) == 0 && deck_count_after_search_started(Card::Oricorio) > 0 &&
           deck_count_after_search_started(Card::Grass) + deck_count_after_search_started(Card::Fire) >= 2;
  }

  Card fss_target_after_search_started() const {
    if (state_.turn == 1 && !scenario_.going_first && should_play_steven() && deck_count_after_search_started(Card::StevensResolve) > 0) return Card::StevensResolve;

    if (need_regi() && bench_space() > 0) {
      if (deck_count_after_search_started(Card::RegidragoV) > 0) return Card::RegidragoV;
      if (prize_count_after_reveal(Card::RegidragoV) > 0 && deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;
    }
    if (need_vstar()) {
      if (deck_count_after_search_started(Card::RegidragoVstar) > 0) return Card::RegidragoVstar;
      if (prize_count_after_reveal(Card::RegidragoVstar) > 0 && deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;
    }
    if (final_prized_energy_for_gladion().has_value() && deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;

    if (need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && deck_count_after_search_started(Card::LatiasEx) > 0) return Card::LatiasEx;
    // Star Alchemy can fetch Tate & Liza when its switch mode fixes Active position
    // while a held Brilliant Blender remains the legal same-turn payload outlet:
    // https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164 https://www.pokemon.com/us/pokemon-tcg/rules
    if (need_active_vstar() && !can_free_retreat_with_latias() && supporter_allowed() && !need_energy() &&
        need_payload() && has_live_blender_payload_line() && deck_count_after_search_started(Card::TateLiza) > 0) {
      return Card::TateLiza;
    }
    if (need_active_vstar() && !need_energy() && !need_payload() &&
        deck_count_after_search_started(Card::TateLiza) > 0) return Card::TateLiza;
    if (need_payload() && payload_might_be_in_deck() && !item_locked() &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0) return Card::BrilliantBlender;
    if (need_payload() && payload_might_be_in_deck() && item_locked() &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0) return Card::ProfessorBurnet;

    // Crispin attaches one selected Energy and puts the other into hand. Promote it
    // over draw-only Energy search only when that exact resolution plus the one manual
    // attachment completes every remaining GGF component this turn: https://api.pokemontcg.io/v2/cards/sv7-133
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const auto crispin_completes_energy_axis_this_turn = [this]() {
      if (!crispin_can_advance_energy_axis()) return false;
      const int grass = grass_needed();
      const int fire = fire_needed();
      const bool grass_available = deck_count_after_search_started(Card::Grass) > 0;
      const bool fire_available = deck_count_after_search_started(Card::Fire) > 0;
      if (grass == 1 && fire == 0) return grass_available && (fire_available || !state_.manual_energy_used);
      if (grass == 0 && fire == 1) return fire_available && (grass_available || !state_.manual_energy_used);
      return grass == 1 && fire == 1 && !state_.manual_energy_used && grass_available && fire_available;
    };
    const auto oricorio_preserves_burnet_payload_line = [this]() {
      const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
      const bool needed_energy_in_deck = (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||
                                         (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);
      const bool oricorio_in_deck = bench_space() > 0 && !in_play(Card::Oricorio) &&
                                    hand_count(Card::Oricorio) == 0 && deck_count_after_search_started(Card::Oricorio) > 0;
      const bool no_payload_in_hand = std::none_of(state_.hand.begin(), state_.hand.end(), is_payload);
      // Star Alchemy may search any card, so it can take Oricorio instead of Crispin.
      // Oricorio then finds the final Basic Energy for the manual attachment, leaving
      // Professor Burnet's Supporter play available for the current-turn payload:
      // https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sm2-55
      // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://www.pokemon.com/us/pokemon-tcg/rules
      return supporter_allowed() && !state_.manual_energy_used && exactly_one_energy_missing &&
             needed_energy_in_deck && oricorio_in_deck && no_payload_in_hand && hand_count(Card::ProfessorBurnet) > 0 &&
             can_play_payload_this_turn() && payload_might_be_in_deck() && !has_live_blender_payload_line();
    };
    if (need_energy() && oricorio_preserves_burnet_payload_line()) return Card::Oricorio;
    if (need_energy() && crispin_completes_energy_axis_this_turn() &&
        deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;
    if (need_energy() && can_fetch_oricorio_for_compression()) return Card::Oricorio;
    if (need_energy() && deck_count_after_search_started(Card::EarthenVessel) > 0 && is_legal_cost_available(can_play_payload_this_turn(), true)) return Card::EarthenVessel;
    if (need_energy() && crispin_can_advance_energy_axis() && deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;
    if (need_energy() && !state_.manual_energy_used) {
      // Star Alchemy may search the deck for any card: https://api.pokemontcg.io/v2/cards/swsh12-156
      // With no stronger Energy connector, take the missing Basic Energy that can be manually attached this turn.
      if (grass_needed() > 0 && deck_count_after_search_started(Card::Grass) > 0) return Card::Grass;
      if (fire_needed() > 0 && deck_count_after_search_started(Card::Fire) > 0) return Card::Fire;
    }
    if (deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    if (deck_count_after_search_started(Card::MysteriousTreasure) > 0) return Card::MysteriousTreasure;
    if (deck_count_after_search_started(Card::QuickBall) > 0) return Card::QuickBall;
    return Card::Serena;
  }
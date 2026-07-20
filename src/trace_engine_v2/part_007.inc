    if (card == Card::TapuLeleGX) {
      if (!ability_available_for_pokemon(Card::TapuLeleGX)) {
        trace("ABILITY BLOCKED", "R-PATH-01", "Tapu Lele-GX Wonder Tag is suppressed by the Rule Box Ability lock.");
        return;
      }
      // Card text source: https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // The search permits inspecting the deck to select a Supporter.
      record_deck_search_knowledge("Wonder Tag");
      const Card target = choose_supporter_after_search_started();
      if (move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("WONDER TAG", "R-TAPU-01", "Searched and revealed " + std::string(name(target)) + ".");
      } else {
        shuffle(state_.deck);
        trace("WONDER TAG", "R-TAPU-01", "No selected Supporter was in deck after legal search.");
      }
      return;
    }
    if (card == Card::Oricorio) {
      // Card text source: https://api.pokemontcg.io/v2/cards/sm2-55
      // Oricorio has no Rule Box; Path-style rule-box suppression does not stop Vital Dance.
      search_energy_to_hand(2, "R-ORICORIO-01", "Vital Dance");
    }
  }

  Card choose_supporter_after_search_started() const {
    // Wonder Tag is resolving after legal deck inspection. A known prized
    // immediate Pokemon axis, or final attachable Basic Energy, makes Gladion
    // the fastest Supporter line.
    const bool prized_regi = need_regi() && deck_count_after_search_started(Card::RegidragoV) == 0 &&
                             prize_count_after_reveal(Card::RegidragoV) > 0;
    const bool prized_vstar = need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) == 0 &&
                              prize_count_after_reveal(Card::RegidragoVstar) > 0;
    if ((prized_regi || prized_vstar || final_prized_energy_for_gladion().has_value()) &&
        deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;

    // Steven's Resolve may take Regidrago V before it enters play, making Wonder Tag
    // a legal turn-one setup connector: https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/swsh12-135
    if (state_.turn == 1 && !scenario_.going_first && should_play_steven() &&
        deck_count_after_search_started(Card::StevensResolve) > 0) return Card::StevensResolve;

    const bool pineco_in_play = in_play_count(Card::Pineco) > 0 ||
        in_play_count(Card::ForretressEx) > 0;
    const bool pineco_available = pineco_in_play || hand_count(Card::Pineco) > 0 ||
        deck_count_after_search_started(Card::Pineco) > 0;
    const bool forretress_available = in_play_count(Card::ForretressEx) > 0 ||
        hand_count(Card::ForretressEx) > 0 ||
        deck_count_after_search_started(Card::ForretressEx) > 0;
    const bool prior_turn_pineco = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::Pineco && pokemon.entered_turn < state_.turn;
        }) || (state_.active && state_.active->card == Card::Pineco &&
               state_.active->entered_turn < state_.turn);
    const bool forest_route = state_.turn > 1 &&
        (state_.stadium == Stadium::ForestOfVitality ||
         hand_count(Card::ForestOfVitality) > 0);
    const bool dawn_timing_live =
        (state_.turn == 1 && !scenario_.going_first) || prior_turn_pineco ||
        forest_route;
    // Dawn can collect Pineco, Forretress ex, and a Stage 2 Dragon in one legal
    // Supporter search. Wonder Tag selects that compression route only when the
    // Pineco Bench slot, evolution timing, and Exploding Energy Ability are live:
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1096
    if (forretress_combo_enabled() && need_energy() &&
        ability_available_for_pokemon(Card::ForretressEx) &&
        deck_count_after_search_started(Card::Dawn) > 0 && pineco_available &&
        forretress_available && dawn_timing_live &&
        (pineco_in_play || bench_space() > 0)) {
      return Card::Dawn;
    }

    const bool held_manual_completes_energy = !state_.manual_energy_used &&
        ((grass_needed() == 1 && fire_needed() == 0 && hand_count(Card::Grass) > 0) ||
         (grass_needed() == 0 && fire_needed() == 1 && hand_count(Card::Fire) > 0));
    const bool current_turn_active_vstar_route = active_is_vstar() ||
        (state_.active && state_.active->card == Card::RegidragoV &&
         state_.active->entered_turn < state_.turn && hand_count(Card::RegidragoVstar) > 0);
    const bool held_payload =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Serena's draw mode may discard a held Dragon payload. When one held Basic
    // Energy and the unused manual attachment already complete GGF, Wonder Tag must
    // select that deterministic current-turn payload Supporter before an unused
    // Crispin Energy connector. This preserves the once-per-game Legacy Star:
    // Wonder Tag: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, evolution, and manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1086
    const bool serena_completes_current_turn_route = strict_payload_timing() &&
        supporter_allowed() && need_payload() && can_play_payload_this_turn() &&
        held_manual_completes_energy && current_turn_active_vstar_route && held_payload &&
        deck_count_after_search_started(Card::Serena) > 0;
    if (serena_completes_current_turn_route) return Card::Serena;

    // Crispin source: https://api.pokemontcg.io/v2/cards/sv7-133
    // Do not choose it after K1 if the inspected deck contains no needed Energy.
    if (need_energy() && crispin_can_advance_energy_axis() && deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;

    // Wonder Tag may search Arven, Arven may search Earthen Vessel, and Vessel may
    // search the final Basic Energy for this turn's unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_in_deck =
        (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);
    const bool arven_vessel_energy_route = need_energy() && exactly_one_energy_missing &&
        !state_.manual_energy_used && !item_locked() && needed_energy_in_deck &&
        deck_count_after_search_started(Card::Arven) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        is_legal_cost_available(can_play_payload_this_turn(), true);
    if (arven_vessel_energy_route) return Card::Arven;

    // After Wonder Tag establishes K1, choose Arven for the target-legal
    // held-payload Item bridge recognized by the preflight. A direct held Item stays
    // preferable because it avoids spending Tapu's Bench slot and the Supporter play:
    // Wonder Tag: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/648
    if (!has_live_one_discard_hand_payload_line() &&
        wonder_tag_arven_held_payload_route_available()) {
      return Card::Arven;
    }

    // Wonder Tag has now inspected the deck. Burnet can only discard cards it
    // selects from that deck, so it is not a payload connector after K1 proves
    // no modeled payload remains: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool deck_payload_available = payload_might_be_in_deck();
    const bool blender_is_playable = deck_payload_available && !item_locked() &&
                                     hand_count(Card::BrilliantBlender) > 0 && can_play_payload_this_turn();
    if (need_payload() && can_play_payload_this_turn() && deck_payload_available && !blender_is_playable &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0) {
      // Burnet directly searches and discards up to 2 cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Wonder Tag must take it when Blender cannot supply this payload line: https://api.pokemontcg.io/v2/cards/sv8-164
      return Card::ProfessorBurnet;
    }
    if (need_vstar() && deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    // Wonder Tag can search Tate & Liza, whose switch mode immediately promotes a
    // Benched Regidrago VSTAR. Select Tate when the target already pays GGF and the
    // payload is ready or a payable held one-discard Item can establish it afterward:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // Serena is only a fallback because it cannot promote the Benched attacker:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_tapu_tate_switch_override.inc#L14-L29
    const bool payload_axis_completable_after_tate = !need_payload() ||
        (state_.deck.size() > 1U && has_live_one_discard_hand_payload_line());
    if (need_active_vstar() && benched_vstar_promotion_ready() &&
        payload_axis_completable_after_tate && !can_free_retreat_with_latias() &&
        deck_count_after_search_started(Card::TateLiza) > 0) {
      return Card::TateLiza;
    }
    if (deck_count_after_search_started(Card::Serena) > 0) return Card::Serena;
    const bool tate_would_promote_incomplete_vstar = need_active_vstar() &&
        !benched_vstar_promotion_ready() && !can_free_retreat_with_latias();
    // Wonder Tag is optional. Do not fall through to Tate's switch route after the
    // target-aware branch rejected an Energy-incomplete promotion:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (deck_count_after_search_started(Card::TateLiza) > 0 &&
        !tate_would_promote_incomplete_vstar) {
      return Card::TateLiza;
    }

    // Wonder Tag may search any Supporter. A completed deck inspection must not
    // nominate an absent hard-coded card while another legal Supporter remains:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    for (const Card fallback : {Card::Dawn, Card::Crispin, Card::Arven,
                                Card::ProfessorBurnet, Card::StevensResolve,
                                Card::Gladion, Card::Lusamine, Card::Guzma,
                                Card::Channeler, Card::TeamYellsCheer,
                                Card::RoseannesBackup, Card::ProfessorTuro}) {
      if (deck_count_after_search_started(fallback) > 0) return fallback;
    }
    // Wonder Tag is optional. Returning an absent sentinel makes the caller
    // resolve the legal no-card case when the deck contains no Supporter.
    return Card::Crispin;
  }

  std::vector<Card> choose_energy_targets_after_search_started(const int max_cards) const {
    std::vector<Card> targets;
    int g_need = grass_needed();
    int f_need = fire_needed();
    int g_available = deck_count_after_search_started(Card::Grass);
    int f_available = deck_count_after_search_started(Card::Fire);
    while (static_cast<int>(targets.size()) < max_cards) {
      Card wanted = Card::Grass;
      if (g_need > 0 && f_need > 0) {
        wanted = std::find(targets.begin(), targets.end(), Card::Grass) == targets.end() ? Card::Grass : Card::Fire;
      } else if (g_need > 0) {
        wanted = Card::Grass;
      } else if (f_need > 0) {
        wanted = Card::Fire;
      } else {
        // Vital Dance and Earthen Vessel each allow up to 2 Basic Energy cards.
        // After GGF is covered, select another legal Basic Energy for deck thinning
        // and potential duplicate-Energy discard capacity: https://api.pokemontcg.io/v2/cards/sm2-55 https://api.pokemontcg.io/v2/cards/sv4-163
        wanted = g_available > 0 ? Card::Grass : Card::Fire;
      }
      if ((wanted == Card::Grass && g_available == 0) || (wanted == Card::Fire && f_available == 0)) {
        const Card other = wanted == Card::Grass ? Card::Fire : Card::Grass;
        if ((other == Card::Grass && g_available == 0) || (other == Card::Fire && f_available == 0)) break;
        wanted = other;
      }
      targets.push_back(wanted);
      if (wanted == Card::Grass) {
        --g_available;
        if (g_need > 0) --g_need;
      } else {
        --f_available;
        if (f_need > 0) --f_need;
      }
    }
    return targets;
  }

  void search_energy_to_hand(const int max_cards, std::string_view rules, std::string_view source) {
    // A legal Energy search reveals the whole deck even when it finds zero cards.
    record_deck_search_knowledge(source);
    const std::vector<Card> targets = choose_energy_targets_after_search_started(max_cards);
    for (const Card target : targets) {
      if (!move_deck_to_hand(target)) throw std::logic_error("energy search target disappeared");
    }
    shuffle(state_.deck);
    trace(std::string(source), rules, "Searched up to " + std::to_string(max_cards) + " Basic Energy: " + join_cards(targets) + ".");
  }

  bool attach_manual() {
    if (state_.manual_energy_used) return false;
    Pokemon* target = target_regi();
    if (target == nullptr || !need_energy()) return false;
    Card wanted = Card::Grass;
    if (grass_needed() > 0 && hand_count(Card::Grass) > 0) {
      wanted = Card::Grass;
    } else if (fire_needed() > 0 && hand_count(Card::Fire) > 0) {
      wanted = Card::Fire;
    } else {
      // Do not use the once-per-turn attachment on an Energy type that does not advance GGF.
      return false;
    }
    remove_one(state_.hand, wanted);
    if (wanted == Card::Grass) ++target->grass; else ++target->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY", std::string(name(wanted)) + " manually to " + std::string(name(target->card)) + ".");
    return true;
  }

  bool evolve_best_regi() {
    if (state_.turn == 1 || hand_count(Card::RegidragoVstar) == 0) return false;
    Pokemon* best = nullptr;

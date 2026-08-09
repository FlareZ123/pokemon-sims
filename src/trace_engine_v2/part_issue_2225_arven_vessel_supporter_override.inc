  std::optional<Card> issue_2225_arven_vessel_payload_cost() const {
    for (const Card card : {Card::Dragapult, Card::Appletun,
                            Card::MegaDragonite, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_2225_arven_vessel_finishing_energy() const {
    if (!state_.active || state_.active->card != Card::RegidragoVstar) {
      return std::nullopt;
    }
    // Earthen Vessel may search either Basic Energy type. Project each physical
    // candidate onto the actual attacker and accept it only when that one manual
    // attachment pays Apex Dragon. DDE therefore makes Grass and Fire alternative
    // one-card finishes rather than additive typed deficits:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Repository DDE contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2426
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (deck_count_after_search_started(basic) == 0) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) && pays_apex_energy_cost(projected)) {
        return basic;
      }
    }
    return std::nullopt;
  }

  bool issue_2225_arven_vessel_finish_available() const {
    return strict_payload_timing() && prizes_known() && supporter_allowed() &&
        !item_locked() && deck_search_available() &&
        !state_.manual_energy_used && active_is_vstar() &&
        need_payload() && hand_count(Card::Arven) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        issue_2225_arven_vessel_finishing_energy().has_value() &&
        issue_2225_arven_vessel_payload_cost().has_value();
  }

  bool play_issue_2225_arven_vessel_finish() {
    if (!issue_2225_arven_vessel_finish_available()) return false;
    const Card payload = *issue_2225_arven_vessel_payload_cost();
    const Card finishing_energy = *issue_2225_arven_vessel_finishing_energy();

    // K1 is already established before this Supporter decision. Arven can spend
    // the one Supporter action on Earthen Vessel, whose mandatory one-card cost
    // discards the held Dragon payload. Vessel then searches the projected Basic
    // for the unused manual attachment, completing both unresolved axes this turn.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Gladion and Oricorio route being preempted: https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sm2-55
    // Official Supporter, Item, discard-cost, deck-search, and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, supporter contention, dynamic DCI/JIT, and earliest-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2225
    // Refined DDE bug: https://github.com/FlareZ123/pokemon-sims/issues/2426
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2225 route");
    if (!move_deck_to_hand(Card::EarthenVessel)) {
      throw std::logic_error("Issue-2225 Earthen Vessel target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Arven searched Earthen Vessel for the complete Energy-plus-payload route.");

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(payload, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-2225 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-2225 route");
    if (!move_deck_to_hand(finishing_energy)) {
      throw std::logic_error("Issue-2225 finishing Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(payload)) +
              " as the current-turn payload and searched " +
              std::string(name(finishing_energy)) +
              " for the unused manual attachment.");
    return true;
  }

  bool issue_2315_tate_switch_finishes_ready_turn() const {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        !need_active_vstar() || state_.manual_energy_used || !payload_ready() ||
        can_free_retreat_with_latias()) {
      return false;
    }

    const Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || pays_apex_energy_cost(*target)) return false;
    const std::array<Card, 2> basic_energies{Card::Grass, Card::Fire};
    const bool held_basic_completes_apex = std::any_of(
        basic_energies.begin(), basic_energies.end(),
        [this, target](const Card energy) {
          if (hand_count(energy) == 0) return false;
          Pokemon projected = *target;
          return attach_energy_card(projected, energy) &&
              pays_apex_energy_cost(projected);
        });
    if (!held_basic_completes_apex) return false;

    // DDE makes Grass and Fire alternative one-card completions, so test each
    // held physical Basic through the actual attachment/payment semantics.
    // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Confirmed DDE migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
    // Tate & Liza's switch mode resolves the only remaining board-position axis,
    // while the already held Basic Energy completes GGF with the unused manual
    // attachment and the current-turn Dragon payload is already established. Its
    // draw mode would shuffle away that deterministic finish and leave the Basic
    // Active stranded, which is exactly the post-Legacy-Star seed-21 failure:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dialga-GX Basic Active witness: https://api.pokemontcg.io/v2/cards/sm5-100
    // Official one-Supporter-per-turn, switch, and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Supporter contention and earliest-complete-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed recovery-and-switch bug: https://github.com/FlareZ123/pokemon-sims/issues/2315
    return true; // Completing held Basic was projected above: https://github.com/FlareZ123/pokemon-sims/issues/2421
  }

  void choose_supporter() {
    if (issue_2315_tate_switch_finishes_ready_turn()) {
      if (!play_tate_switch()) {
        throw std::logic_error("Issue-2315 Tate switch finish became unavailable");
      }
      return;
    }
    if (play_issue_2225_arven_vessel_finish()) return;
    choose_supporter_issue2225_original();
  }
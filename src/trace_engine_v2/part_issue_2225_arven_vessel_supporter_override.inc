  std::optional<Card> issue_2225_arven_vessel_payload_cost() const {
    for (const Card card : {Card::Dragapult, Card::Appletun,
                            Card::MegaDragonite, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_2225_arven_vessel_finish_available() const {
    return strict_payload_timing() && prizes_known() && supporter_allowed() &&
        !item_locked() && deck_search_available() &&
        !state_.manual_energy_used && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        need_payload() && hand_count(Card::Arven) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        issue_2225_arven_vessel_payload_cost().has_value();
  }

  bool play_issue_2225_arven_vessel_finish() {
    if (!issue_2225_arven_vessel_finish_available()) return false;
    const Card payload = *issue_2225_arven_vessel_payload_cost();

    // K1 is already established before this Supporter decision. Arven can spend
    // the one Supporter action on Earthen Vessel, whose mandatory one-card cost
    // discards the held Dragon payload. Vessel then searches the missing Grass for
    // the unused manual attachment, completing both unresolved axes this turn.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Gladion and Oricorio route being preempted: https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sm2-55
    // Official Supporter, Item, discard-cost, deck-search, and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, supporter contention, dynamic DCI/JIT, and earliest-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2225
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
    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-2225 Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded " + std::string(name(payload)) +
              " as the current-turn payload and searched Grass Energy for the unused manual attachment.");
    return true;
  }

  bool issue_2265_common_arven_quick_ball_projection() const {
    const bool jit_profile = scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::MatchupFlexJit;
    // A held Dragon already gives Quick Ball the cheaper direct JIT-payload route.
    // In that state forcing Arven plus Blender spends extra resources and can make
    // Blender correctly decline after Quick Ball itself satisfies the payload axis:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Existing held-payload Quick Ball route: https://github.com/FlareZ123/pokemon-sims/issues/2199
    // Resource-preservation priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug refined by CI: https://github.com/FlareZ123/pokemon-sims/issues/2265
    const bool held_payload =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    return jit_profile && !held_payload && prizes_known() && supporter_allowed() &&
        !item_locked() && state_.turn <= scenario_.max_turn && !state_.retreat_used &&
        state_.active && is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV &&
        hand_count(Card::Arven) > 0 && hand_count(Card::QuickBall) == 0 &&
        hand_count(Card::BrilliantBlender) > 0 && hand_count(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::QuickBall) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck();
  }

  bool issue_2265_hold_treasure_for_direct_route() const {
    if (!issue_2265_common_arven_quick_ball_projection() ||
        state_.manual_energy_used || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Fire) == 0 ||
        !need_active_vstar() || !need_energy() || !need_payload()) {
      return false;
    }
    const Pokemon* target = target_regi();
    if (target == nullptr || target->card != Card::RegidragoV ||
        target->entered_turn >= state_.turn || target->double_dragon != 0 ||
        target->grass < 2 || target->fire != 0) {
      return false;
    }

    // The public K1 state already contains every component of the direct T3 line.
    // Preserve Mysterious Treasure before the scheduled Fire attachment because
    // spending it on Tapu Lele-GX and Wonder Tag cannot improve the complete
    // Arven -> Quick Ball -> Latias ex -> Brilliant Blender route. The held Grass
    // becomes Quick Ball's dynamic-DCI cost only after Fire completes GGF.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core attachment, evolution, Supporter, Item, Bench, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Resource preservation, K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
    return true;
  }

  bool issue_2265_pre_evolution_route_live() const {
    if (!issue_2265_common_arven_quick_ball_projection() ||
        hand_count(Card::RegidragoVstar) == 0 || need_regi() || !need_vstar() ||
        need_energy() || !need_active_vstar() || !need_payload()) {
      return false;
    }
    const Pokemon* target = target_regi();
    return target != nullptr && target->card == Card::RegidragoV &&
        target->entered_turn < state_.turn && target->grass >= 2 &&
        target->fire >= 1;
  }

  bool issue_2265_arven_quick_ball_surplus_route_live() const {
    if (!issue_2265_common_arven_quick_ball_projection() || need_regi() ||
        need_vstar() || need_energy() || !need_active_vstar() || !need_payload()) {
      return false;
    }

    // Arven consumes the turn's Supporter permission, so only the already-held
    // Brilliant Blender may supply the current-turn payload after the search.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // One-Supporter-per-turn and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    return benched_vstar_promotion_ready() &&
        quick_ball_final_surplus_energy_cost().has_value();
  }

  bool play_issue_2265_arven_quick_ball_surplus_route() {
    if (!issue_2265_arven_quick_ball_surplus_route_live()) return false;

    // Search only the Item required by the proven continuation. Arven says to search
    // for an Item and a Pokémon Tool; a typed deck search may choose fewer matching
    // cards, so the Tool branch can legally fail when it advances no setup axis.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Typed-search voluntary failure ruling: https://compendium.pokegym.net/compendium-bw.html#trainers-in-general
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2265 route");
    const bool found = move_deck_to_hand(Card::QuickBall);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01; P-COMPRESS-01",
          found
              ? "Arven searched Quick Ball for the final surplus-Energy Latias/Blender route."
              : "Arven legally inspected the deck, but the projected Quick Ball target disappeared.");
    return true;
  }

  bool complete_issue_2265_arven_quick_ball_surplus_route() {
    if (issue_2265_pre_evolution_route_live()) {
      evolve_best_regi();
    }
    if (!issue_2265_arven_quick_ball_surplus_route_live() ||
        !play_issue_2265_arven_quick_ball_surplus_route()) {
      return false;
    }
    if (!play_quick_ball(false)) {
      throw std::logic_error("Issue-2265 Quick Ball continuation disappeared");
    }
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("Issue-2265 Latias ex Bench action disappeared");
    }
    if (!play_brilliant_blender()) {
      throw std::logic_error("Issue-2265 Brilliant Blender payload disappeared");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("Issue-2265 Skyliner promotion disappeared");
    }

    // The complete deterministic line ends with GGF Regidrago VSTAR Active and a
    // Dragon payload entering discard this turn. No Tapu Lele-GX, Wonder Tag,
    // Mysterious Treasure, or extra Supporter resource is consumed by this route.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Resource-preserving earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
    return active_is_vstar() && !need_energy() && !need_payload();
  }

  void choose_supporter() {
    if (issue_2265_pre_evolution_route_live() ||
        issue_2265_arven_quick_ball_surplus_route_live()) {
      // The direct Arven line is a complete current-turn route. It must preempt the
      // extra Treasure -> Tapu Lele-GX -> Wonder Tag branch because that branch
      // spends more cards and Bench space without improving the ready turn.
      // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Supporter, Item, discard, Bench, Ability, evolution, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // K1, supporter contention, dynamic DCI/JIT, and earliest-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
      if (!complete_issue_2265_arven_quick_ball_surplus_route()) {
        throw std::logic_error("Issue-2265 direct Arven route became unavailable");
      }
      return;
    }
    if (play_issue_2225_arven_vessel_finish()) return;
    choose_supporter_issue2225_original();
  }

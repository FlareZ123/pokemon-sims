  bool tate_draw_after_active_vstar_evolution_route() const {
    // Under strict JIT, a prior-turn Active Regidrago V that already has GGF cannot
    // improve its Energy or ready-turn payload axes with Celestial Roar. When Star
    // Alchemy has placed Regidrago VSTAR in hand during Item lock, evolve before
    // Tate & Liza replaces the hand with five cards:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Evolution, Supporter, search, shuffle, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest legal route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1162
    return supporter_allowed() && strict_payload_timing() && item_locked() &&
           hand_count(Card::TateLiza) > 0 &&
           hand_count(Card::RegidragoVstar) > 0 && state_.active &&
           state_.active->card == Card::RegidragoV &&
           state_.active->entered_turn < state_.turn &&
           state_.active->grass >= 2 && state_.active->fire >= 1;
  }

  bool tate_switch_after_held_vstar_evolution_route() const {
    const bool payload_in_known_deck = !deck_seen_ ||
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || item_locked() ||
        !state_.active || state_.active->card == Card::RegidragoVstar ||
        !payload_in_known_deck || can_free_retreat_with_latias()) {
      return false;
    }

    return std::any_of(state_.bench.begin(), state_.bench.end(),
                       [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
          pokemon.fire >= 1;
    });
  }

  bool tate_switch_after_incomplete_held_vstar_evolution_route() const {
    if (!supporter_allowed() || !strict_payload_timing() ||
        hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Serena) == 0 ||
        !state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX ||
        can_free_retreat_with_latias() || !need_energy()) {
      return false;
    }

    // A prior-turn Benched Regidrago V at GF can evolve now, and Tate & Liza can
    // then switch that VSTAR Active. Preserve those two held, deterministic axes
    // before considering Tate's random five-card redraw:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Evolution, switching, attachment, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest direct route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1197
    return std::any_of(state_.bench.begin(), state_.bench.end(),
                       [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn && pokemon.grass == 1 &&
          pokemon.fire >= 1;
    });
  }

  bool tate_switch_after_incomplete_vstar_evolution_ready() const {
    if (!supporter_allowed() || !strict_payload_timing() ||
        hand_count(Card::TateLiza) == 0 || hand_count(Card::Serena) == 0 ||
        !state_.active ||
        state_.active->card != Card::TapuLeleGX ||
        can_free_retreat_with_latias() || need_vstar() ||
        !need_active_vstar() || !need_energy()) {
      return false;
    }

    // The fixed-point policy has now evolved the preserved Basic. Use Tate's
    // printed switch mode before Serena or Tate draw can randomize that progress:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter and switching procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1197
    return std::any_of(state_.bench.begin(), state_.bench.end(),
                       [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar &&
          pokemon.entered_turn < state_.turn && pokemon.grass == 1 &&
          pokemon.fire >= 1;
    });
  }

  std::optional<Card> issue_1700_pre_steven_vessel_cost() const {
    const bool searchable_fire =
        deck_count_after_search_started(Card::Fire) > 0;
    if (searchable_fire) {
      for (const Card card : {Card::Appletun, Card::MegaDragonite,
                              Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
      return std::nullopt;
    }
    if (strict_payload_timing()) {
      return hand_count(Card::RegidragoVstar) > 0
          ? std::optional<Card>{Card::RegidragoVstar}
          : std::nullopt;
    }
    if (hand_count(Card::MysteriousTreasure) > 0) {
      return Card::MysteriousTreasure;
    }
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1700_pre_steven_vessel_route_available() const {
    // Full Prize inspection and a resolved deck search establish the same K1
    // knowledge for this deterministic pre-Steven route:
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1926
    if (!prizes_known() || !supporter_allowed() ||
        item_locked() || scenario_.locks != LockMode::None ||
        state_.turn != 1 || scenario_.going_first ||
        state_.turn + 2 > scenario_.max_turn || state_.manual_energy_used ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || bench_space() == 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        !issue_1700_pre_steven_vessel_cost().has_value()) {
      return false;
    }

    const bool current_turn_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn == state_.turn && pokemon.grass == 0 &&
              pokemon.fire == 0;
        });
    if (!current_turn_regidrago ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) < 2) {
      return false;
    }

    const bool searchable_fire =
        deck_count_after_search_started(Card::Fire) > 0;
    if (searchable_fire) {
      // Vessel can establish the first attachment before Steven ends T1. Steven then
      // searches Latias ex and the second Grass, while held VSTAR and Blender preserve
      // the evolution and current-turn payload axes for the deterministic T3 finish:
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core Item, Energy, Supporter, evolution, retreat, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // K1, DCI, AMR, supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
      return deck_count_after_search_started(Card::Fire) > 0 &&
          payload_might_be_in_deck();
    }

    const bool fire_is_known_prized =
        prize_count_after_reveal(Card::Fire) > 0;
    const bool replacement_vstar_available =
        !strict_payload_timing() ||
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
    const bool payload_axis_available =
        payload_ready() || payload_might_be_in_deck();

    // When every Fire is known prized, two Grass searches plus the current attachment
    // let Steven bank Latias and Gladion. Strict and matchup-flex routes spend the held
    // VSTAR only when Steven can replace it; no-discard-control spends a route-replaced
    // Treasure or payload. Gladion then recovers Fire on T2 and Blender supplies the
    // ready-turn Dragon payload on T3 when required:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Core Item, Prize, Energy, Supporter, evolution, retreat, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and multi-axis route replacement: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
    return fire_is_known_prized && replacement_vstar_available &&
        payload_axis_available &&
        deck_count_after_search_started(Card::Gladion) > 0;
  }

  bool play_issue_1700_pre_steven_vessel_route() {
    if (!issue_1700_pre_steven_vessel_route_available()) return false;
    const std::optional<Card> cost = issue_1700_pre_steven_vessel_cost();
    if (!cost) return false;

    const bool searchable_fire =
        deck_count_after_search_started(Card::Fire) > 0;
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*cost, "Earthen Vessel issue-1700 route cost",
                           "R-EV-01; P-DCI-01")) {
      throw std::logic_error("Issue-1700 Vessel cost disappeared");
    }
    if (!move_deck_to_hand(Card::Grass) ||
        !(searchable_fire ? move_deck_to_hand(Card::Fire)
                          : move_deck_to_hand(Card::Grass))) {
      throw std::logic_error("Issue-1700 Vessel Energy target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          searchable_fire
              ? "Earthen Vessel searched Grass and Fire before Steven's Resolve."
              : "Earthen Vessel searched two Grass before Steven's Resolve; Fire is known prized.");

    auto setup_regi = std::find_if(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn == state_.turn && pokemon.grass == 0 &&
              pokemon.fire == 0;
        });
    if (setup_regi == state_.bench.end() || !remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue-1700 attachment target disappeared");
    }
    ++setup_regi->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          "Grass Energy manually to Regidrago V before Steven's Resolve.");

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1700 route");

    std::vector<Card> found;
    if (!searchable_fire && strict_payload_timing()) {
      if (!move_deck_to_hand(Card::RegidragoVstar)) {
        throw std::logic_error("Issue-1700 replacement VSTAR disappeared");
      }
      found.push_back(Card::RegidragoVstar);
    }
    if (!move_deck_to_hand(Card::LatiasEx)) {
      throw std::logic_error("Issue-1700 Latias target disappeared");
    }
    found.push_back(Card::LatiasEx);
    if (searchable_fire) {
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("Issue-1700 second Grass target disappeared");
      }
      found.push_back(Card::Grass);
    } else {
      if (!move_deck_to_hand(Card::Gladion)) {
        throw std::logic_error("Issue-1700 Gladion target disappeared");
      }
      found.push_back(Card::Gladion);
    }
    shuffle(state_.deck);

    // Steven says "up to 3 cards", so each branch selects exactly its missing axes.
    // All printed effects and game actions are linked beside the executable planner:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1700
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Searched the complete post-Vessel T3 route: " + join_cards(found) +
              ". Turn ended.");
    return true;
  }

  void choose_supporter() {
    if (play_issue_1700_pre_steven_vessel_route()) return; // Earthen Vessel, Steven, and confirmed route: https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/sm7-145 https://github.com/FlareZ123/pokemon-sims/issues/1700
    if (play_issue_1700_known_prize_gladion_before_blender()) return; // Gladion and earliest T3 route: https://api.pokemontcg.io/v2/cards/sm4-95 https://github.com/FlareZ123/pokemon-sims/issues/1700

    if (play_issue_1771_steven_t4_package()) return;
    if (play_issue_1772_steven_t3_package()) return;
    if (issue_1772_t3_crispin_completion_available()) {
      if (!play_crispin()) {
        throw std::logic_error("Known issue-1772 Crispin completion failed");
      }
      return;
    }
    if (tate_switch_after_incomplete_held_vstar_evolution_route()) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1197-tate-before-held-vstar-evolution", state_.turn,
            "HOLD SUPPORTER", "R-TATE-01; R-RVS-01; P-COMPRESS-01",
            "Retained Tate & Liza until the held Regidrago VSTAR evolves the prior-turn GF Benched Basic.");
      }
      return;
    }
    if (tate_switch_after_incomplete_vstar_evolution_ready()) {
      if (!play_tate_switch()) {
        throw std::logic_error("Known issue-1197 Tate switch route failed");
      }
      return;
    }
    if (tate_draw_after_active_vstar_evolution_route()) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1162-tate-after-active-vstar-search", state_.turn,
            "HOLD SUPPORTER", "R-TATE-01; R-FSS-02; R-RVS-01; P-JIT-01; P-COMPRESS-01",
            "Retained Tate & Liza until the held Regidrago VSTAR evolves the prior-turn GGF Active.");
      }
      return;
    }
    if (tate_switch_after_held_vstar_evolution_route()) {
      // Star Alchemy has already placed Regidrago VSTAR in hand. Preserve Tate & Liza
      // through the current fixed-point pass so the prior-turn GGF Basic evolves first;
      // Tate's switch mode then promotes it and the held Brilliant Blender supplies the
      // current-turn JIT payload. The concrete public state is sufficient even before
      // generic VSTAR-axis helpers recognize the unevolved Benched attacker:
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Supporter, evolution, switching, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Strict and matchup-flex JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1070
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1070-tate-after-vstar-search", state_.turn,
            "HOLD SUPPORTER", "R-TATE-01; R-RVS-01; P-COMPRESS-01",
            "Retained Tate & Liza until the held Regidrago VSTAR evolves the prepared Benched Basic.");
      }
      return;
    }
    choose_supporter_issue1070_original();
  }

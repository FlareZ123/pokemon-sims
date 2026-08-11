  bool issue_3173_steven_route_available() const {
    const Pokemon* regidrago = nullptr;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card == Card::RegidragoV && pokemon.grass == 1 &&
          pokemon.fire == 0 && !pays_apex_energy_cost(pokemon)) {
        regidrago = &pokemon;
        break;
      }
    }

    // Steven ends the present turn, so admission is governed by the physical
    // continuation on the following turn. Project that public turn boundary and
    // ask the shared lock predicates rather than matching one historical lock ID.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official turn, Supporter, Item, evolution, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Scenario-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    Engine next_turn = *this;
    ++next_turn.state_.turn;
    next_turn.state_.turn_ended = false;
    const bool next_turn_actions_legal = next_turn.supporter_allowed() &&
        !next_turn.item_locked() &&
        next_turn.ability_available_for_pokemon(Card::LatiasEx);

    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);
    const bool active_can_use_skyliner = state_.active &&
        is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV;

    // Both JIT profiles require the Dragon payload on the ready turn. Treasure
    // supplies that discard on the projected ready turn, so the physical route is
    // shared by StrictJit and MatchupFlexJit. NoDiscardControl remains excluded by
    // strict_payload_timing(). A Regidrago V already on the Bench now may evolve on
    // the following turn even when it entered play during this Steven turn.
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Official evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    return strict_payload_timing() && state_.turn + 1 <= scenario_.max_turn &&
        prizes_known() && !state_.turn_ended && supporter_allowed() &&
        state_.manual_energy_used && !state_.retreat_used &&
        active_can_use_skyliner && regidrago != nullptr && bench_space() > 0 &&
        !in_play(Card::LatiasEx) && next_turn_actions_legal &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 && deck_payload &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
  }

  bool play_issue_3173_banked_steven() {
    if (!issue_3173_steven_route_available()) return false;

    const auto payload_it = std::find_if(
        state_.deck.begin(), state_.deck.end(), is_payload);
    if (payload_it == state_.deck.end()) {
      throw std::logic_error("Issue-3173 Steven payload target disappeared");
    }
    const Card payload = *payload_it;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-3173 semantic route");
    if (!move_deck_to_hand(Card::RegidragoVstar) ||
        !move_deck_to_hand(Card::Crispin) || !move_deck_to_hand(payload)) {
      throw std::logic_error("Issue-3173 Steven package target disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    issue_1796_finish_turn_ = state_.turn + 1;

    // Steven may search any three cards and then ends the turn. Preserve the held
    // Treasure because its mandatory discard is the ready-turn Dragon outlet.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter and deck-search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Banked Regidrago VSTAR, Crispin, and " + std::string(name(payload)) +
              " for the semantic next-turn Treasure and Latias ex finish. Turn ended.");
    return true;
  }

  bool complete_issue_3173_finish() {
    if (issue_1796_finish_turn_ != state_.turn) return false;

    const auto payload_it = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool evolvable_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass == 1 &&
              pokemon.fire == 0 && !pays_apex_energy_cost(pokemon);
        });
    const bool active_can_use_skyliner = state_.active &&
        is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV;

    // Resolve only the action legality the continuation actually consumes. This
    // admits Rule-Box-unlocked and later-turn equivalents while real Supporter,
    // Item, Ability, Bench, Retreat, evolution, and horizon failures still block it.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official Supporter, Item, evolution, Ability, Bench, attachment, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Scenario-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    const bool route_available = strict_payload_timing() &&
        state_.turn <= scenario_.max_turn && prizes_known() &&
        !state_.turn_ended && supporter_allowed() && !state_.manual_energy_used &&
        !item_locked() && !state_.retreat_used && active_can_use_skyliner &&
        evolvable_regidrago && bench_space() > 0 && !in_play(Card::LatiasEx) &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        payload_it != state_.hand.end() &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    if (!route_available) {
      issue_1796_finish_turn_ = 0;
      return false;
    }
    const Card payload = *payload_it;

    evolve_best_regi();
    const bool benched_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar;
        });
    if (!benched_vstar) {
      throw std::logic_error("Issue-3173 Regidrago V did not evolve");
    }

    // Crispin may attach one of two different Basic Energy from the deck; the
    // ordinary manual attachment supplies the other missing Basic. These actions
    // precede Treasure so the Dragon discard occurs on the same ready turn.
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Energy attachment and Trainer procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    if (need_energy()) {
      if (!play_crispin()) {
        throw std::logic_error("Issue-3173 Crispin Energy package disappeared");
      }
      if (!attach_manual()) {
        throw std::logic_error("Issue-3173 manual Energy attachment disappeared");
      }
    }

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(payload, "Mysterious Treasure issue-3173 Dragon cost",
                           "R-MT-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-3173 Treasure payload cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-3173 finish");
    if (!move_deck_to_hand(Card::LatiasEx)) {
      throw std::logic_error("Issue-3173 Latias ex target disappeared");
    }
    shuffle(state_.deck);
    trace("MYSTERIOUS TREASURE", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-JIT-01",
          "Discarded " + std::string(name(payload)) +
              " and searched Latias ex for the Active-position axis.");
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("Issue-3173 Latias ex became unbenchable");
    }
    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("Issue-3173 Skyliner promotion became unavailable");
    }
    issue_1796_finish_turn_ = 0;

    // Skyliner removes the Basic Active's Retreat Cost while Latias ex remains in
    // play. Treasure's mandatory cost has already supplied the current-turn Dragon,
    // so the promoted Regidrago VSTAR now has all Apex Dragon setup axes.
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Ability and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    return active_is_vstar() && pays_apex_energy_cost(*state_.active) &&
        payload_ready();
  }

  void choose_supporter_issue_3173() {
    if (play_issue_3173_banked_steven()) return;
    choose_supporter_issue_1152();
  }

  void run_turn_issue_3173() {
    // A banked Steven continuation is a deterministic earliest-ready route. Resolve
    // it before generic Items, attachments, or Supporters can consume its reserved
    // channels on the projected turn.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
    if (complete_issue_3173_finish()) {
      trace("POLICY", "P-COMPRESS-01; P-JIT-01",
            "Completed the semantic Steven, Treasure, and Latias ex route.");
      return;
    }
    // Defer lookup of the later-composed Latias turn member through a dependent
    // generic lambda. This keeps textual include order intact while allowing the
    // fallback member to be declared later in the same Engine class.
    // C++ dependent member lookup: https://eel.is/c++draft/temp.dep
    // Confirmed composition fix: https://github.com/FlareZ123/pokemon-sims/issues/3173
    auto run_legacy = []<typename EngineLike>(EngineLike* engine) {
      engine->run_turn_issue_3173_legacy();
    };
    run_legacy(this);
  }


  bool discard_from_hand(const Card card, std::string_view why, std::string_view rules) {
    if (!remove_one(state_.hand, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    trace("DISCARD", rules, std::string(name(card)) + " (" + std::string(why) + ")");
    return true;
  }

  // Tracks a card leaving the discard pile in the same turn, so strict-JIT state
  // cannot count a payload that was discarded and then recovered before the ready check.
  bool recover_discard_to_hand(const Card card) {
    const auto discard_it = std::find(state_.discard.begin(), state_.discard.end(), card);
    if (discard_it == state_.discard.end()) return false;
    state_.hand.push_back(*discard_it);
    state_.discard.erase(discard_it);
    const auto turn_it = std::find(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), card);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    return true;
  }

  bool bench_from_hand(const Card card, const bool resolve_entry) {
    if (bench_space() <= 0 || !is_basic(card) || !remove_one(state_.hand, card)) return false;
    state_.bench.push_back(Pokemon{card, state_.turn});
    trace("BENCH", "R-GAME-BENCH", std::string(name(card)) + " from hand.");
    if (resolve_entry) resolve_entry_ability(card);
    return true;
  }

  bool vstar_can_exist_this_turn() const {
    if (has_vstar()) return true;
    if (state_.turn <= 1) return false;
    const auto can_evolve_now = [this](const Pokemon& pokemon) {
      // A Pokémon cannot evolve during the turn it entered play:
      // https://www.pokemon.com/us/pokemon-tcg/rules
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
    };
    if (state_.active && can_evolve_now(*state_.active)) return true;
    return std::any_of(state_.bench.begin(), state_.bench.end(), can_evolve_now);
  }

  bool sole_known_payload_is_in_hand() const {
    if (!prizes_known()) return false;
    const int payloads_in_hand = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    // K1 fixed-list knowledge proves whether another modeled payload remains in
    // deck. Preserve the sole in-hand payload when no replacement is searchable:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return payloads_in_hand == 1 && !payload_might_be_in_deck();
  }

  bool can_play_payload_this_turn() const {
    if (!payload_window_open() || !need_payload()) return false;
    if (!strict_payload_timing()) return true;
    // Strict JIT recognizes a Dragon payload only on the same player turn that
    // creates readiness. A payload cannot advance that contract unless a
    // Regidrago VSTAR already exists or a prior-turn Regidrago V may evolve now:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/721
    return vstar_can_exist_this_turn();
  }

  // This is a policy knowledge check. Before a legal deck or Prize inspection it
  // relies only on public copies; after K1 it respects the inspected deck state.
  // Brilliant Blender and Professor Burnet can only place deck cards in discard:
  // https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  bool payload_might_be_in_deck() const {
    return might_be_unseen(Card::Appletun) || might_be_unseen(Card::MegaDragonite) ||
           might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar) ||
           might_be_unseen(Card::DialgaGX);
  }

  bool has_live_blender_payload_line() const {
  // A strict-JIT Blender route is live only when its one-use deck discard can
  // accompany a current-turn Regidrago VSTAR card route. Higher-level Supporter,
  // switching, and attachment selectors must use the same boundary as the action:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  return !item_locked() && hand_count(Card::BrilliantBlender) > 0 &&
         can_play_payload_this_turn() && payload_might_be_in_deck() &&
         blender_vstar_card_axis_can_finish_this_turn();
}

  bool can_spend_redundant_payload_for_locked_vstar_search() const {
    const int held_payloads = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    const bool burnet_outlet = hand_count(Card::ProfessorBurnet) > 0 &&
        payload_might_be_in_deck();
    const bool serena_outlet = hand_count(Card::Serena) > 0;

    // Before a scheduled turn-two Item lock, a direct Pokémon search may spend one
    // redundant Dragon payload to secure Regidrago VSTAR while another payload stays
    // protected and a Supporter-based strict-JIT outlet survives the lock. Ordinary
    // low-DCI costs remain preferred; callers use this only as a fallback:
    // Mysterious Treasure https://api.pokemontcg.io/v2/cards/sm6-113
    // Ultra Ball https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Regidrago VSTAR https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Serena https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/925
    return scenario_.dci == DciProfile::StrictJit &&
           scenario_.locks == LockMode::TurnTwoItem && state_.turn == 1 &&
           !need_regi() && need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
           might_be_unseen(Card::RegidragoVstar) && held_payloads >= 2 &&
           (burnet_outlet || serena_outlet);
  }

  std::optional<Card> choose_discard(const bool permit_payload, const bool flex_fodder,
                                     const bool allow_heavy_ball = true,
                                     const std::optional<Card> excluded_from_cost = std::nullopt,
                                     const bool allow_redundant_payload = false) const {
    // P-DCI-STRICT and P-DCI-FLEX, documented in RULES_TRACEABILITY.md.
    const auto has_discardable_copy = [this, excluded_from_cost](const Card card) {
      const int excluded = excluded_from_cost && *excluded_from_cost == card ? 1 : 0;
      return hand_count(card) > excluded;
    };
    if (permit_payload) {
      for (const Card card : {Card::Appletun, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }
    if (allow_redundant_payload) {
      const int held_payloads = static_cast<int>(std::count_if(
          state_.hand.begin(), state_.hand.end(), is_payload));
      if (held_payloads >= 2) {
        for (const Card card : {Card::Appletun, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
          if (hand_count(card) > 0) return card;
        }
      }
    }

    // Dipplin cannot be legally put into play without Applin, which is absent from this deck.
    if (hand_count(Card::Dipplin) > 0) return Card::Dipplin;

    // Grant and Wishful Baton have no setup-axis value in these goldfish recipes.
    // Grant remains recoverable from discard through its printed auxiliary effect,
    // while Wishful Baton requires an opponent attack Knock Out that this model does
    // not simulate:
    // https://api.pokemontcg.io/v2/cards/swsh10-144
    // https://api.pokemontcg.io/v2/cards/sm3-128
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    if (has_discardable_copy(Card::Grant)) return Card::Grant;
    if (has_discardable_copy(Card::WishfulBaton)) return Card::WishfulBaton;

    if (hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) >= 2) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && has_vstar()) return Card::RegidragoVstar;

    const bool final_energy_vessel_completion =
        scenario_.dci == DciProfile::NoDiscardControl &&
        excluded_from_cost && *excluded_from_cost == Card::EarthenVessel &&
        !state_.manual_energy_used && active_is_vstar() && payload_ready() &&
        grass_needed() + fire_needed() == 1 &&
        ((grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
         (fire_needed() == 1 && might_be_unseen(Card::Fire)));
    if (final_energy_vessel_completion) {
      // Once Active Regidrago VSTAR, payload, and two Energy are complete,
      // these held roles no longer advance the repository setup objective.
      // Earthen Vessel may therefore spend one before searching the final
      // Basic Energy for the unused manual attachment:
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1050
      for (const Card card : {Card::QuickBall, Card::StevensResolve,
                              Card::LatiasEx, Card::Gladion}) {
        if (has_discardable_copy(card)) return card;
      }
    }

    const bool setup_before_steven_route =
        excluded_from_cost && *excluded_from_cost == Card::MysteriousTreasure &&
        state_.turn == 1 && !scenario_.going_first && !item_locked() &&
        supporter_allowed() && need_regi() && bench_space() > 0 &&
        can_free_retreat_with_latias() && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::Fire) == 1 && hand_count(Card::Grass) > 0 &&
        might_be_unseen(Card::RegidragoV) && might_be_unseen(Card::RegidragoVstar) &&
        might_be_unseen(Card::Crispin) && might_be_unseen(Card::BrilliantBlender) &&
        might_be_unseen(Card::Grass) && might_be_unseen(Card::Fire);
    if (setup_before_steven_route && flex_fodder &&
        scenario_.dci == DciProfile::MatchupFlexJit &&
        has_discardable_copy(Card::ErikasInvitation)) {
      // Erika's Invitation is the canonical low-impact matchup-flex cost. Using it
      // preserves the public Fire Energy while the same Mysterious Treasure,
      // Steven's Resolve, Crispin, Latias ex, and Brilliant Blender route still
      // reaches the identical turn-two ready state:
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/issues/915
      return Card::ErikasInvitation;
    }
    if (setup_before_steven_route) {
      // Spending the public Fire Energy lets Mysterious Treasure establish
      // Regidrago V before Steven's Resolve ends turn one. Grass remains for the
      // manual attachment, Crispin restores GGF on turn two, and Latias ex supplies
      // the proven promotion route:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/775
      return Card::Fire;
    }

    // Extra in-hand Energy is observable; no deck / Prize identities are used to call Energy expendable.
    if (hand_count(Card::Grass) > grass_needed() + 1) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed() + 1) return Card::Fire;

    const int held_payloads_for_turo = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    const bool latias_is_live_active_connector =
        ability_available_for_pokemon(Card::LatiasEx) &&
        (hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx));
    const bool active_has_retreat_energy = state_.active &&
        state_.active->grass + state_.active->fire > 0;
    const bool turo_is_only_observable_active_connector = state_.active &&
        state_.active->card == Card::TapuLeleGX &&
        !latias_is_live_active_connector && hand_count(Card::TateLiza) == 0 &&
        !active_has_retreat_energy;
    const bool preserve_turo_for_stranded_active =
        scenario_.dci == DciProfile::MatchupFlexJit && flex_fodder &&
        excluded_from_cost && *excluded_from_cost == Card::MysteriousTreasure &&
        !item_locked() && state_.turn == 1 && scenario_.going_first &&
        need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV) &&
        hand_count(Card::ProfessorTuro) > 0 && held_payloads_for_turo >= 2 &&
        turo_is_only_observable_active_connector;
    if (preserve_turo_for_stranded_active) {
      // One of two held Dragon payloads retains attack value after becoming the
      // Mysterious Treasure cost, while singleton Professor Turo remains the only
      // public route that can clear the stranded Active Tapu Lele-GX:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sv4-171
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/1205
      for (const Card card : {Card::Appletun, Card::MegaDragonite,
                              Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }

    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
      // Erika's Invitation replaced Mawile-GX in the canonical deck. Its effect is
      // opponent-dependent and deliberately inert in this goldfish model, so it is
      // the current low-impact matchup-flex discard candidate:
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // https://github.com/FlareZ123/pokemon-sims/issues/852
      for (const Card card : {Card::Grant, Card::WishfulBaton, Card::ErikasInvitation, Card::Guzma, Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                              Card::RoseannesBackup, Card::Lusamine, Card::ProfessorTuro,
                              Card::Powerglass}) {
        if (hand_count(card) > 0) return card;
      }
    }

    if (scenario_.dci == DciProfile::NoDiscardControl) {
      for (const Card card : {Card::Appletun, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }

    // Hisuian Heavy Ball is a legal generic discard cost, but K0 policy must
    // resolve its Prize-information effect first. Card text source:
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    const bool known_no_basic_prize = prizes_known() &&
        std::none_of(state_.prizes.begin(), state_.prizes.end(), is_basic);
    if (allow_heavy_ball && known_no_basic_prize && hand_count(Card::HisuianHeavyBall) > 0) {
      return Card::HisuianHeavyBall;
    }

    // The card currently being played cannot pay its own discard cost. When an
    // effect identifies that card, a distinct Ultra Ball is another card in hand
    // that may pay Quick Ball's discard cost: https://api.pokemontcg.io/v2/cards/swsh1-179
    if (excluded_from_cost && has_discardable_copy(Card::UltraBall)) return Card::UltraBall;
    // Mysterious Treasure says to discard a card from hand, so a distinct second
    // copy remains a legal cost for the first copy: https://api.pokemontcg.io/v2/cards/sm6-113
    if (hand_count(Card::MysteriousTreasure) >= 2) return Card::MysteriousTreasure;

    // Earthen Vessel requires discarding another card from hand, so a distinct
    // second copy remains a legal cost for the first copy: https://api.pokemontcg.io/v2/cards/sv4-163
    if (hand_count(Card::EarthenVessel) >= 2) return Card::EarthenVessel;

    // A second Quick Ball is "another card" for the first Quick Ball's discard
    // cost, so it remains legal low-DCI fodder when no higher-priority cost exists:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    if (hand_count(Card::QuickBall) >= 2) return Card::QuickBall;

    // Strict DCI permits one duplicated baseline Supporter to pay a discard cost
    // while another copy remains for the turn's single Supporter play. Existing
    // duplicate search Items stay ahead because they consume fewer future options:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sm4-95
    for (const Card card : {Card::Arven, Card::Crispin, Card::Gladion}) {
      if (hand_count(card) >= 2) return card;
    }

    // Quick Ball may discard any other card from hand. When two Tapu Lele-GX are
    // held, discarding one preserves the same future Wonder Tag Supporter connector.
    // Keep this behind every existing duplicate and surplus category so the fallback
    // cannot expose a protected singleton or consume lower-DCI fuel first:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/711
    if (hand_count(Card::TapuLeleGX) >= 2) return Card::TapuLeleGX;
    return std::nullopt;
  }

  bool is_legal_cost_available(const bool permit_payload, const bool flex_fodder) const {
    return choose_discard(permit_payload, flex_fodder).has_value();
  }

  std::optional<std::array<Card, 2>> redundant_payload_ultra_ball_vstar_costs() {
    if (!can_spend_redundant_payload_for_locked_vstar_search()) return std::nullopt;
    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };

    // Ultra Ball needs two other cards. Spend exactly one redundant payload, then
    // require a separate ordinary DCI-legal cost so one payload remains protected:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/925
    const auto payload = choose_discard(false, true, true, Card::UltraBall, true);
    if (!payload || !is_payload(*payload) || !remove_one(state_.hand, *payload)) {
      restore_hand();
      return std::nullopt;
    }
    const auto ordinary = choose_discard(false, true, true, Card::UltraBall);
    restore_hand();
    if (!ordinary) return std::nullopt;
    return std::array<Card, 2>{*payload, *ordinary};
  }

  bool oricorio_preserves_burnet_supporter_slot() const {
    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_might_be_in_deck =
        (grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
        (fire_needed() == 1 && might_be_unseen(Card::Fire));
    const bool no_payload_in_hand = std::none_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Vital Dance can search the final Basic Energy for a manual attachment,
    // preserving the one Supporter play for Professor Burnet's deck-to-discard
    // payload line: https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://www.pokemon.com/us/pokemon-tcg/rules
    return supporter_allowed() && !state_.manual_energy_used && exactly_one_energy_missing &&
           needed_energy_might_be_in_deck && no_payload_in_hand && can_play_payload_this_turn() &&
           hand_count(Card::ProfessorBurnet) > 0 && !has_live_blender_payload_line() && payload_might_be_in_deck();
  }

  // P-CONNECTOR-01: do not spend a search connector for Oricorio while a present Supporter or Item
  // already supplies the needed Energy axis this turn.
  bool needs_oricorio_connector() const {
    if (!need_energy()) return false;
    // Vital Dance can search the final Basic Energy for a manual attachment (https://api.pokemontcg.io/v2/cards/sm2-55),
    // preserving the Supporter play for Professor Burnet's deck-to-discard payload line (https://api.pokemontcg.io/v2/cards/swsh12tg-TG26).
    const bool oricorio_preserves_burnet = oricorio_preserves_burnet_supporter_slot();
    if (oricorio_preserves_burnet) return true;
    // Crispin can only be the preferred connector when its Supporter slot is not
    // needed by the direct Burnet payload route: https://api.pokemontcg.io/v2/cards/sv7-133
    if (supporter_allowed() && hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis()) return false;
    if (!item_locked() && hand_count(Card::EarthenVessel) > 0 && is_legal_cost_available(can_play_payload_this_turn(), true)) return false;
    // Regidrago V requires two Grass and one Fire. An off-type Energy cannot pay a
    // missing component, while Vital Dance may search the exact Basic type: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/sm2-55
    return hand_count(Card::Grass) < grass_needed() || hand_count(Card::Fire) < fire_needed();
  }

  bool payable_ultra_ball_vstar_route() {
    // Ultra Ball directly searches Regidrago VSTAR only when its two other-card
    // discard costs are actually payable: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (item_locked() || hand_count(Card::UltraBall) == 0 || !need_vstar() ||
        !might_be_unseen(Card::RegidragoVstar)) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    const bool permit_payload = can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Reserve the Ultra Ball being played. Two distinct remaining copies may pay
    // its two-other-card cost: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const auto first = choose_discard(permit_payload, true, true, Card::UltraBall);
    if (first && remove_one(state_.hand, *first)) {
      const auto second = choose_discard(can_play_payload_this_turn() && !is_payload(*first),
                                         true, true, Card::UltraBall);
      restore_hand();
      if (second) return true;
    } else {
      restore_hand();
    }
    return redundant_payload_ultra_ball_vstar_costs().has_value();
  }

  bool wonder_tag_arven_held_payload_route_available() const {
    // At K1, Wonder Tag may find Arven, Arven may find a one-discard Item, and that
    // Item may discard the held Dragon only when its mandatory search retains a
    // legal target. Arven is a Supporter and is still in deck during target choice:
    // Wonder Tag: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Trainer resolution ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // Strict-JIT contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/648
    if (!deck_seen_ || item_locked() || !can_play_payload_this_turn() ||
        !need_payload() || need_regi() || need_vstar() || need_energy() ||
        need_active_vstar() || hand_count(Card::Arven) > 0 ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    const bool treasure_route = deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    const bool quick_route = deck_count_after_search_started(Card::QuickBall) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);
    const bool vessel_route = deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        (deck_count_after_search_started(Card::Grass) > 0 ||
         deck_count_after_search_started(Card::Fire) > 0);
    return treasure_route || quick_route || vessel_route;
  }

  bool needs_tapu_connector() {
    if (!supporter_allowed() || !ability_available_for_pokemon(Card::TapuLeleGX)) return false;
    // With only one Bench slot left, Tapu Lele-GX into Crispin consumes both the
    // Bench slot and the single Supporter play. Oricorio can instead find the final
    // Energy and leave Professor Burnet playable for the current-turn payload:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (bench_space() == 1 && hand_count(Card::Oricorio) > 0 && !in_play(Card::Oricorio) &&
        oricorio_preserves_burnet_supporter_slot()) {
      return false;
    }
    // Steven's Resolve may fetch the absent Basic setup card on turn one going second:
    // https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/swsh12-135
    const bool needs_t1_steven = state_.turn == 1 && !scenario_.going_first && should_play_steven() &&
                                 hand_count(Card::StevensResolve) == 0;
    const bool direct_communication_oricorio_route = need_energy() &&
        pokemon_communication_has_direct_target_route(Card::Oricorio);
    const bool needs_crispin = need_energy() && !direct_communication_oricorio_route &&
        (hand_count(Card::Crispin) == 0 || !crispin_can_advance_energy_axis());
    // Wonder Tag can inspect the deck for Burnet. It is a live direct payload
    // bridge only while a payload may remain in deck: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    const bool needs_burnet = can_play_payload_this_turn() && payload_might_be_in_deck() &&
                              hand_count(Card::ProfessorBurnet) == 0 && !has_live_blender_payload_line() &&
                              might_be_unseen(Card::ProfessorBurnet);
    // A payable Ultra Ball already searches Regidrago VSTAR directly, so Wonder Tag
    // must not spend a Bench slot and Supporter route on indirect Arven:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sv1-166
    const bool direct_ultra_vstar_route = payable_ultra_ball_vstar_route();
    const bool direct_communication_vstar_route = need_vstar() &&
        pokemon_communication_has_direct_target_route(Card::RegidragoVstar);
    const bool needs_arven_for_vstar = need_vstar() && hand_count(Card::Arven) == 0 &&
                                       hand_count(Card::EvolutionIncense) == 0 &&
                                       hand_count(Card::MysteriousTreasure) == 0 &&
                                       !direct_ultra_vstar_route &&
                                       !direct_communication_vstar_route;
    // A direct in-hand one-discard Item avoids Tapu Lele-GX's Bench cost and the
    // Supporter play, so Wonder Tag is reserved for the known Arven-only bridge:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool needs_arven_for_held_payload = bench_space() > 0 &&
        !has_live_one_discard_hand_payload_line() &&
        wonder_tag_arven_held_payload_route_available();
    return needs_t1_steven || needs_crispin || needs_burnet ||
           needs_arven_for_vstar || needs_arven_for_held_payload;
  }

  bool bench_oricorio_if_useful() {
    if (hand_count(Card::Oricorio) == 0 || in_play(Card::Oricorio) || bench_space() <= 0 || !needs_oricorio_connector()) return false;
    return bench_from_hand(Card::Oricorio, true);
  }

  bool bench_tapu_if_useful() {
    if (hand_count(Card::TapuLeleGX) == 0 || in_play(Card::TapuLeleGX) || bench_space() <= 0 || !needs_tapu_connector()) return false;
    return bench_from_hand(Card::TapuLeleGX, true);
  }

  void resolve_entry_ability(const Card card) {
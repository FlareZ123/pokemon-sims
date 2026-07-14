
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
    // A prior-turn Regidrago V may evolve this turn. If none exists, only the
    // K1-proven sole remaining payload is protected; K0 may still spend one of
    // several possible payloads to establish the future Regidrago line:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    return vstar_can_exist_this_turn() || !sole_known_payload_is_in_hand();
  }

  // This is a policy knowledge check. Before a legal deck or Prize inspection it
  // relies only on public copies; after K1 it respects the inspected deck state.
  // Brilliant Blender and Professor Burnet can only place deck cards in discard:
  // https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  bool payload_might_be_in_deck() const {
    return might_be_unseen(Card::MegaDragonite) || might_be_unseen(Card::Dragapult) ||
           might_be_unseen(Card::GoodraVstar) || might_be_unseen(Card::DialgaGX);
  }

  bool has_live_blender_payload_line() const {
    // Brilliant Blender searches and discards cards from deck: https://api.pokemontcg.io/v2/cards/sv8-164
    return !item_locked() && hand_count(Card::BrilliantBlender) > 0 && can_play_payload_this_turn() &&
           payload_might_be_in_deck();
  }

  std::optional<Card> choose_discard(const bool permit_payload, const bool flex_fodder,
                                     const bool allow_heavy_ball = true,
                                     const std::optional<Card> excluded_from_cost = std::nullopt) const {
    // P-DCI-STRICT and P-DCI-FLEX, documented in RULES_TRACEABILITY.md.
    const auto has_discardable_copy = [this, excluded_from_cost](const Card card) {
      const int excluded = excluded_from_cost && *excluded_from_cost == card ? 1 : 0;
      return hand_count(card) > excluded;
    };
    if (permit_payload) {
      for (const Card card : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
        if (hand_count(card) > 0) return card;
      }
    }

    // Dipplin cannot be legally put into play without Applin, which is absent from this deck.
    if (hand_count(Card::Dipplin) > 0) return Card::Dipplin;

    if (hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) >= 2) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && has_vstar()) return Card::RegidragoVstar;

    // Extra in-hand Energy is observable; no deck / Prize identities are used to call Energy expendable.
    if (hand_count(Card::Grass) > grass_needed() + 1) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed() + 1) return Card::Fire;

    if (flex_fodder && scenario_.dci != DciProfile::StrictJit) {
      for (const Card card : {Card::MawileGX, Card::Guzma, Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak, Card::TeamYellsCheer,
                              Card::RoseannesBackup, Card::Lusamine, Card::ProfessorTuro,
                              Card::Powerglass}) {
        if (hand_count(card) > 0) return card;
      }
    }

    if (scenario_.dci == DciProfile::NoDiscardControl) {
      for (const Card card : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
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
    return std::nullopt;
  }

  bool is_legal_cost_available(const bool permit_payload, const bool flex_fodder) const {
    return choose_discard(permit_payload, flex_fodder).has_value();
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
    if (!first || !remove_one(state_.hand, *first)) {
      restore_hand();
      return false;
    }
    const auto second = choose_discard(can_play_payload_this_turn() && !is_payload(*first),
                                       true, true, Card::UltraBall);
    restore_hand();
    return second.has_value();
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
    const bool needs_crispin = need_energy() && (hand_count(Card::Crispin) == 0 || !crispin_can_advance_energy_axis());
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
    const bool needs_arven = need_vstar() && hand_count(Card::Arven) == 0 &&
                             hand_count(Card::EvolutionIncense) == 0 &&
                             hand_count(Card::MysteriousTreasure) == 0 &&
                             !direct_ultra_vstar_route;
    return needs_t1_steven || needs_crispin || needs_burnet || needs_arven;
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
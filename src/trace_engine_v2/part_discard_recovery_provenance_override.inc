  bool recover_discard_to_hand(const Card card) {
    const int discard_copies = count_of(state_.discard, card);
    if (discard_copies == 0) return false;
    const int current_turn_copies = count_of(state_.discarded_this_turn, card);

    const auto discard_it = std::find(state_.discard.begin(), state_.discard.end(), card);
    state_.hand.push_back(*discard_it);
    state_.discard.erase(discard_it);

    // When an older copy exists, recovery can select it and preserve the current-turn
    // copy in discard. Remove provenance only when every matching discard copy entered
    // discard this turn:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/934
    if (current_turn_copies > 0 && discard_copies == current_turn_copies) {
      const auto turn_it = std::find(
          state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), card);
      state_.discarded_this_turn.erase(turn_it);
    }
    return true;
  }


  std::optional<Card> quick_ball_dialga_vessel_route_cost() const {
    // Once Active Regidrago VSTAR already has GGF and the only missing axis is a
    // current-turn Dragon payload, Quick Ball may use a route-conditioned cost,
    // search Basic Dialga-GX, and let a separately held Earthen Vessel discard it.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
    // Opponent-action boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
    // Official Item, discard, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1237
    const bool complete_public_route =
        scenario_.dci == DciProfile::StrictJit && deck_seen_ && !item_locked() &&
        active_is_vstar() && !need_regi() && !need_vstar() && !need_energy() &&
        !need_active_vstar() && need_payload() &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::EarthenVessel) > 0 &&
        deck_count_after_search_started(Card::DialgaGX) > 0 &&
        (deck_count_after_search_started(Card::Grass) > 0 ||
         deck_count_after_search_started(Card::Fire) > 0);
    if (!complete_public_route) return std::nullopt;

    if (hand_count(Card::Channeler) > 0) return Card::Channeler;
    if (hand_count(Card::Grass) > grass_needed()) return Card::Grass;
    if (hand_count(Card::Fire) > fire_needed()) return Card::Fire;
    return std::nullopt;
  }

  std::optional<Card> choose_discard_issue_1237(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard(permit_payload, flex_fodder,
                                         allow_heavy_ball, excluded_from_cost,
                                         allow_redundant_payload);
    if (ordinary || excluded_from_cost != Card::QuickBall) return ordinary;
    return quick_ball_dialga_vessel_route_cost();
  }

  bool has_payable_payload_outlet_after_costed_search_issue_1237(
      const Card excluded) {
    if (has_payable_payload_outlet_after_costed_search(excluded)) return true;
    return excluded == Card::QuickBall &&
           quick_ball_dialga_vessel_route_cost().has_value();
  }

// Limit the route-conditioned wrappers to Mysterious Treasure and Quick Ball.
// The macros are removed at the start of part_009b2.inc before Ultra Ball logic.
// https://github.com/FlareZ123/pokemon-sims/issues/1237
#define choose_discard choose_discard_issue_1237
#define has_payable_payload_outlet_after_costed_search \
  has_payable_payload_outlet_after_costed_search_issue_1237

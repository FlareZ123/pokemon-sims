  bool powerglass_jit_vessel_finish_visible() const {
    if (!strict_payload_timing() || item_locked() || !need_payload() ||
        !need_energy() || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        hand_count(Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        !deck_search_available() || !earthen_vessel_has_legal_target()) {
      return false;
    }

    const bool powerglass_available =
        state_.active->tool == Tool::Powerglass ||
        (state_.active->tool == Tool::None && hand_count(Card::Powerglass) > 0);
    if (!powerglass_available) return false;

    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    const bool exactly_one_basic_energy_missing =
        missing_grass + missing_fire == 1;
    const bool powerglass_target_in_discard =
        (missing_grass == 1 && count_of(state_.discard, Card::Grass) > 0) ||
        (missing_fire == 1 && count_of(state_.discard, Card::Fire) > 0);

    // Earthen Vessel may discard the held Dragon during the current strict-JIT
    // turn, while Powerglass supplies the final Basic Energy from discard at the
    // end of that same turn. The repository objective observes the resulting legal
    // board state, so an older policy that delays Vessel to the following turn is
    // strictly slower whenever this complete route is already public:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, Tool, and end-of-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Ready-state objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
    // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2834
    return exactly_one_basic_energy_missing && powerglass_target_in_discard;
  }

  bool play_powerglass_jit_vessel_finish() {
    if (!powerglass_jit_vessel_finish_visible()) return false;

    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    if (payload == state_.hand.end()) return false;
    const Card discarded_payload = *payload;

    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error("Issue-2834 Earthen Vessel disappeared");
    }
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(discarded_payload,
                           "Earthen Vessel current-turn Powerglass JIT cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-2834 Dragon payload cost disappeared");
    }

    // The Dragon is DCI 1 on this route because the same Vessel payment completes
    // the current-turn payload axis, and Vessel still resolves its printed Basic
    // Energy search before the already-proven Powerglass end-step attachment:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2834
    search_energy_to_hand(
        2, "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01; P-COMPRESS-01",
        "Earthen Vessel issue-2834 Powerglass finish");
    return true;
  }

  bool play_earthen_vessel(const bool permit_payload) {
    if (permit_payload && play_powerglass_jit_vessel_finish()) return true;
    return play_earthen_vessel_issue2834_original(permit_payload);
  }

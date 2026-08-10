#undef issue_1605_arven_crobat_route_available
#include "part_issue_2645_arven_crobat_state_override.inc"

  bool issue_2323_protect_final_t1_payload() const {
    if (scenario_.dci != DciProfile::StrictJit || scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        !prizes_known() || can_play_payload_this_turn()) {
      return false;
    }

    const int held_payloads = payload_cards_in_hand_count();
    const int spent_payloads_this_turn = static_cast<int>(std::count_if(
        state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(),
        is_payload));
    const bool timer_started_this_turn =
        (state_.active && state_.active->card == Card::RegidragoV &&
         state_.active->entered_turn == state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      return pokemon.card == Card::RegidragoV &&
                             pokemon.entered_turn == state_.turn;
                    });

    // The early Mysterious Treasure route has already spent one of two held Dragon
    // payloads, searched and played Regidrago V, and established K1. The generic
    // current-turn need_payload() axis is already satisfied by that first discard,
    // but strict-JIT cannot credit it toward the later turn when the new Regidrago V
    // can evolve and attack. The final held Dragon therefore remains UDP until a
    // same-turn readiness outlet exists. This guard only filters a generic discard
    // candidate when exactly one protected payload remains; every ordinary
    // non-payload DCI candidate selected by the existing policy keeps its original
    // precedence.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Official Item-cost, search, Bench, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2323
    return held_payloads == 1 && spent_payloads_this_turn >= 1 &&
           timer_started_this_turn;
  }

  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto candidate = choose_discard_issue2323_original(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    if (candidate && is_payload(*candidate) &&
        issue_2323_protect_final_t1_payload()) {
      // A printed discard cost is optional because the Trainer itself can simply
      // remain unplayed. Refuse a candidate that would spend the final protected
      // Dragon before strict-JIT can legally use it on the ready turn.
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Strict-JIT and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2323
      return std::nullopt;
    }
    return candidate;
  }

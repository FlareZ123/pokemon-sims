#undef issue_1605_arven_crobat_route_available
#include "part_issue_2645_arven_crobat_state_override.inc"

  bool issue_2323_protect_final_t1_payload() const {
    if (scenario_.dci != DciProfile::StrictJit || scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        !prizes_known() || can_play_payload_this_turn()) {
      return false;
    }

    const int held_payloads = payload_cards_in_hand_count();
    const int spent_payloads_this_turn =
        payload_card_count(state_.discarded_this_turn);
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

  bool issue_3040_professor_turo_is_unique_active_escape() const {
    if (!prizes_known() || !state_.active ||
        hand_count(Card::ProfessorTuro) == 0 || !supporter_allowed()) {
      return false;
    }

    // Preserve Turo only from an observable K1 state where the current Active is a
    // real setup blocker, cannot retreat for free, carries no Energy to pay Retreat,
    // and a Benched Regidrago line is waiting to become the modeled attacker.
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Dialga-GX printed Retreat Cost 3 witness: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Retreat and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and discrete-resource preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
    if (state_.active->card == Card::RegidragoV ||
        state_.active->card == Card::RegidragoVstar ||
        retreat_cost(state_.active->card) == 0 || state_.active->grass > 0 ||
        state_.active->fire > 0 || state_.active->double_dragon > 0) {
      return false;
    }

    const bool benched_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV ||
                 pokemon.card == Card::RegidragoVstar;
        });
    if (!benched_regidrago) return false;

    const bool latias_is_live_active_connector =
        ability_available_for_pokemon(Card::LatiasEx) &&
        (hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx));
    const bool tate_is_live_active_connector = hand_count(Card::TateLiza) > 0;
    return !latias_is_live_active_connector && !tate_is_live_active_connector;
  }

  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto candidate = choose_discard_issue2323_original(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    if (candidate && *candidate == Card::ProfessorTuro &&
        issue_3040_professor_turo_is_unique_active_escape()) {
      const int held_payloads = payload_cards_in_hand_count();
      if (held_payloads >= 2) {
        // One of two held Dragon payloads has higher DCI than the unique Active
        // escape because another payload remains protected for the actual ready-turn
        // JIT event. Ask the existing DCI selector for that redundant-payload cost.
        // Earthen Vessel discard cost: https://api.pokemontcg.io/v2/cards/sv4-163
        // Mysterious Treasure discard cost: https://api.pokemontcg.io/v2/cards/sm6-113
        // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
        // Regidrago VSTAR / same-ready-turn payload objective: https://api.pokemontcg.io/v2/cards/swsh12-136
        // JIT and dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
        const auto replacement = choose_discard_issue2323_original(
            permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
            true);
        if (replacement && is_payload(*replacement)) return replacement;
      }

      // A Trainer with a discard cost may remain unplayed. When no safe alternate
      // cost exists, refusing Turo is preferable to destroying the only observable
      // Active-position connector.
      // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
      // Dynamic DCI / resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
      return std::nullopt;
    }
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
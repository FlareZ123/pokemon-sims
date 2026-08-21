  // Keep the pure route-classification owner separate from Engine state/action code.
  // C++ textual include semantics: https://eel.is/c++draft/cpp.include
  // Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/quick_ball/legacy_star_projection_policy.inc
#include "quick_ball/legacy_star_projection_policy.inc"

  bool quick_ball_has_live_connector_route() {
    const bool basic_connector_live = need_regi() || need_active_vstar() ||
        needs_tapu_connector() || needs_oricorio_connector();
    const bool payload_search_chain = might_be_unseen(Card::DialgaGX) &&
        has_payable_payload_outlet_after_costed_search(Card::QuickBall);
    const LegacyStarQuickBallPolicy::Projection projection{
        false, false, basic_connector_live, payload_search_chain};
    return LegacyStarQuickBallPolicy::connector_route_live(projection);
  }

  bool quick_ball_is_inert_before_legacy_star() {
    const bool can_attempt_quick_ball =
        !item_locked() && hand_count(Card::QuickBall) > 0 && need_payload();
    if (!can_attempt_quick_ball) return false;

    const bool can_jit_discard = can_play_payload_this_turn() &&
        blender_vstar_card_axis_can_finish_this_turn() &&
        payload_zone_contains(state_.hand);
    if (can_jit_discard) return false;

    // Quick Ball remains a live connector only when it can establish a missing
    // Basic-Pokémon axis or a searched Dialga-GX still has a payable later outlet.
    // Once held Crispin has completed GGF and the Active VSTAR axis, a held Quick
    // Ball with none of those routes cannot replace Legacy Star's discard-seven:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
    const LegacyStarQuickBallPolicy::Projection projection{
        can_attempt_quick_ball, can_jit_discard,
        quick_ball_has_live_connector_route(), false};
    return LegacyStarQuickBallPolicy::inert_before_legacy_star(projection);
  }

  bool use_legacy_star() {
    if (!quick_ball_is_inert_before_legacy_star()) {
      return use_legacy_star_issue1016_original();
    }

    const int held_quick_balls = hand_count(Card::QuickBall);
    std::erase(state_.hand, Card::QuickBall);
    const bool used = use_legacy_star_issue1016_original();
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_quick_balls),
                       Card::QuickBall);

    // The held Quick Ball is temporarily excluded only from route evaluation. It
    // stays in hand physically while Legacy Star resolves and remains preserved:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
    return used;
  }

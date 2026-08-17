  // Keep route-local projection bookkeeping in one stateless owner while this
  // wrapper retains DCI/JIT admission and the physical Legacy Star action.
  // Canonical projection owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/legacy_star/quick_ball_projection_policy.inc
  // C++ textual include semantics: https://eel.is/c++draft/cpp.include
#include "legacy_star/quick_ball_projection_policy.inc"

  class LegacyStarQuickBallPolicy final {
   public:
    [[nodiscard]] static bool connector_route_live(
        const bool basic_connector_live, const bool payload_search_chain) {
      return basic_connector_live || payload_search_chain;
    }

    [[nodiscard]] static bool inert_before_legacy_star(
        const bool can_attempt_quick_ball, const bool can_jit_discard,
        const bool has_live_connector_route) {
      // Quick Ball requires another card discarded from hand, while Legacy Star
      // discards the top 7 cards of the deck. A held Quick Ball is strategically
      // inert here only after current-turn JIT use and all live connector routes
      // have been ruled out: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      return can_attempt_quick_ball && !can_jit_discard &&
             !has_live_connector_route;
    }
  };

  bool quick_ball_has_live_connector_route() {
    const bool basic_connector_live = need_regi() || need_active_vstar() ||
        needs_tapu_connector() || needs_oricorio_connector();
    const bool payload_search_chain = might_be_unseen(Card::DialgaGX) &&
        has_payable_payload_outlet_after_costed_search(Card::QuickBall);
    return LegacyStarQuickBallPolicy::connector_route_live(
        basic_connector_live, payload_search_chain);
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
    return LegacyStarQuickBallPolicy::inert_before_legacy_star(
        can_attempt_quick_ball, can_jit_discard,
        quick_ball_has_live_connector_route());
  }

  bool use_legacy_star() {
    if (!quick_ball_is_inert_before_legacy_star()) {
      return use_legacy_star_issue1016_original();
    }

    const int held_quick_balls =
        LegacyStarQuickBallProjectionPolicy::hide_from_route_view(state_.hand);
    const bool used = use_legacy_star_issue1016_original();
    LegacyStarQuickBallProjectionPolicy::restore_to_hand(state_.hand,
                                                          held_quick_balls);

    // The held Quick Ball is temporarily excluded only from route evaluation. It
    // stays in hand physically while Legacy Star resolves and remains preserved:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Projection owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/legacy_star/quick_ball_projection_policy.inc
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
    return used;
  }
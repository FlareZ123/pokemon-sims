  class LegacyStarQuickBallPolicy final {
   public:
    struct Projection final {
      bool can_attempt_quick_ball;
      bool can_jit_discard;
      bool basic_connector_live;
      bool payload_search_chain;

      [[nodiscard]] bool has_live_connector_route() const {
        return basic_connector_live || payload_search_chain;
      }
    };

    [[nodiscard]] static bool connector_route_live(const Projection& projection) {
      return projection.has_live_connector_route();
    }

    [[nodiscard]] static bool inert_before_legacy_star(const Projection& projection) {
      // Quick Ball requires another card discarded from hand, while Legacy Star
      // discards the top 7 cards of the deck. A held Quick Ball is strategically
      // inert here only after current-turn JIT use and all live connector routes
      // have been ruled out: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      return projection.can_attempt_quick_ball && !projection.can_jit_discard &&
             !projection.has_live_connector_route();
    }
  };

  bool issue_1683_rulebox_locked_quick_ball_is_serena_cost() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        !rule_box_abilities_locked() || hand_count(Card::QuickBall) != 1 ||
        need_regi() || in_play_count(Card::RegidragoV) +
                           in_play_count(Card::RegidragoVstar) < 2 ||
        !in_play(Card::Oricorio)) {
      return false;
    }

    // Test the held Quick Ball against the exact public hand and K1 deck state.
    // When its printed discard cost and every setup-advancing Basic route both fail,
    // the singleton is setup-dead under the current Rule Box lock and is lower DCI
    // than Serena, Earthen Vessel, recovery, Energy, VSTAR, or a strict-JIT payload:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Core Supporter, Item, Ability, search, discard, and Bench procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest productive route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Engine's implicitly copied reference member continues to refer to the same
    // object, so preserve the live RNG and suppress trace output around this silent
    // projection: https://eel.is/c++draft/class.copy.ctor#15
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
    const std::mt19937_64 live_rng = rng_;
    Engine projected = *this;
    projected.trace_ = nullptr;
    const bool quick_ball_has_route = projected.play_quick_ball(false);
    rng_ = live_rng;
    return !quick_ball_has_route;
  }

  bool play_serena_permanent_item_lock_if_selected(
      const bool allow_zero_draw_payload_completion) {
    const bool permanent_full_item_lock =
        scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined;
    if (!permanent_full_item_lock || scenario_.dci != DciProfile::StrictJit ||
        !supporter_allowed() || hand_count(Card::Serena) == 0) {
      return false;
    }

    // Project only the existing Serena selector. The issue #1855 fix may improve
    // optional costs after that selector accepts the play, while it must not open
    // a new Serena line or replace the selector's mandatory first discard:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Full Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Engine copy and RNG-reference behavior: https://eel.is/c++draft/class.copy.ctor#15
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1855
    const std::size_t discard_before = state_.discard.size();
    const std::mt19937_64 live_rng = rng_;
    Engine projected = *this;
    projected.trace_ = nullptr;
    const bool selected = projected.play_serena_issue1683_original(
        allow_zero_draw_payload_completion);
    rng_ = live_rng;
    if (!selected || projected.state_.discard.size() < discard_before + 2U ||
        projected.state_.discard[discard_before] != Card::Serena) {
      return false;
    }

    std::vector<Card> original_plan;
    const std::size_t projected_end = std::min(
        projected.state_.discard.size(), discard_before + 4U);
    for (std::size_t index = discard_before + 1U;
         index < projected_end; ++index) {
      original_plan.push_back(projected.state_.discard[index]);
    }
    if (original_plan.empty()) return false;

    const Card first = original_plan.front();
    const bool permit_payload = serena_payload_can_complete_this_turn();
    const std::vector<Card> original_hand = state_.hand;
    std::vector<Card> planned_discards{first};
    remove_one(state_.hand, Card::Serena);
    if (!remove_one(state_.hand, first)) {
      state_.hand = original_hand;
      return false;
    }

    const auto projected_draws = [this]() {
      const std::size_t missing = state_.hand.size() < 5U
          ? 5U - state_.hand.size()
          : 0U;
      return std::min(missing, state_.deck.size());
    };
    std::size_t original_extra = 1U;
    for (int number = 0; number < 2; ++number) {
      std::optional<Card> chosen;
      const auto locked_item = std::find_if(
          state_.hand.begin(), state_.hand.end(), is_item);
      if (locked_item != state_.hand.end()) {
        const std::size_t draws_before = projected_draws();
        const std::size_t hand_after_discard = state_.hand.size() - 1U;
        const std::size_t missing_after = hand_after_discard < 5U
            ? 5U - hand_after_discard
            : 0U;
        const std::size_t draws_after =
            std::min(missing_after, state_.deck.size());
        if (draws_after > draws_before) chosen = *locked_item;
      }
      while (!chosen && original_extra < original_plan.size()) {
        const Card candidate = original_plan[original_extra++];
        if (!is_payload(candidate) && hand_count(candidate) > 0) {
          chosen = candidate;
        }
      }
      if (!chosen || !remove_one(state_.hand, *chosen)) break;
      planned_discards.push_back(*chosen);
    }

    const bool would_draw = state_.hand.size() < 5U && !state_.deck.empty();
    const bool completes_payload_now = allow_zero_draw_payload_completion &&
        is_payload(first) && permit_payload;
    state_.hand = original_hand;
    if (!would_draw && !completes_payload_now) return false;

    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    for (std::size_t index = 0; index < planned_discards.size(); ++index) {
      discard_from_hand(
          planned_discards[index],
          index == 0U ? "Serena chosen discard" : "Serena optional discard",
          "R-SERENA-01");
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) {
      draw_card("Serena");
    }
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER",
          "Used Serena draw mode after replacing permanently Item-locked optional costs.");
    return true;
  }

  bool play_serena(const bool allow_zero_draw_payload_completion = false) {
    if (play_serena_permanent_item_lock_if_selected(
            allow_zero_draw_payload_completion)) {
      return true;
    }
    if (play_serena_issue1683_original(allow_zero_draw_payload_completion)) {
      return true;
    }
    if (!supporter_allowed() || hand_count(Card::Serena) == 0 ||
        !issue_1683_rulebox_locked_quick_ball_is_serena_cost()) {
      return false;
    }

    // Serena must discard at least one card before drawing to five. In this
    // confirmed locked public state, Quick Ball has no productive legal route,
    // so spending it increases the current turn's setup probability while every
    // live singleton and unresolved axis remains protected:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Official Supporter, Item, Ability, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    discard_from_hand(Card::QuickBall, "Serena chosen discard", "R-SERENA-01");
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER",
          "Used Serena draw mode after discarding setup-dead Quick Ball under Rule Box lock.");
    return true;
  }

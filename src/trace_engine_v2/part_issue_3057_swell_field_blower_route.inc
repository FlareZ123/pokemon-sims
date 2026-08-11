  bool issue_3057_forest_pineco_route_visible() const {
    // Forest of Vitality may replace the modeled Path to the Peak directly and
    // then permit a current-turn Grass Pineco to evolve after the first turn.
    // This shared predicate describes the Pineco/Forretress Energy axis only;
    // the direct and deferred callers separately prove how Forest is obtained.
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Stadium / evolution / Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Confirmed merged-scope bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (!rule_box_abilities_locked() || state_.stadium_used || state_.turn <= 1 ||
        !forretress_combo_enabled() || !has_any_regi() || !need_energy() ||
        !might_be_unseen(Card::Grass)) {
      return false;
    }

    const auto prior_turn_pineco = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::Pineco &&
          pokemon.entered_turn < state_.turn;
    };
    const bool pineco_can_evolve_normally =
        (state_.active && prior_turn_pineco(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), prior_turn_pineco);
    if (pineco_can_evolve_normally) return false;

    const bool current_turn_pineco =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn == state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      return pokemon.card == Card::Pineco &&
                          pokemon.entered_turn == state_.turn;
                    });
    const bool pineco_can_enter_this_turn =
        in_play_count(Card::Pineco) == 0 && bench_space() > 0 &&
        (hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco));
    const bool forretress_available =
        hand_count(Card::ForretressEx) > 0 || might_be_unseen(Card::ForretressEx);
    return forretress_available &&
        (current_turn_pineco || pineco_can_enter_this_turn);
  }

  bool issue_3064_direct_forest_route_available() const {
    // A held Forest is itself the shortest legal Path-removal connector. Playing
    // Field Blower first spends an Item without advancing another axis, so direct
    // Forest dominates whenever the same-turn Pineco line is live. Item lock does
    // not prohibit Stadium cards.
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Core Stadium / Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return issue_3057_forest_pineco_route_visible() &&
        hand_count(Card::ForestOfVitality) > 0;
  }

  bool issue_3064_deferred_forest_route_available() const {
    // When Forest is absent from hand, Field Blower may be the correct first
    // connector only if the exact post-Blower public state proves a payable Secret
    // Box route. Require K1 so the Forest target is known to be in deck, then reuse
    // Secret Box's full-axis and three-card DCI planner after physically removing
    // Field Blower from the projected hand. This keeps access distinct from success.
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Core Item / Stadium / search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Secret Box ALS and DCI planner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1118_secret_box.inc
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (!issue_3057_forest_pineco_route_visible() || item_locked() ||
        hand_count(Card::FieldBlower) == 0 ||
        hand_count(Card::ForestOfVitality) > 0 ||
        hand_count(Card::SecretBox) == 0 || !prizes_known() ||
        deck_count_after_search_started(Card::ForestOfVitality) == 0) {
      return false;
    }

    std::mt19937_64 projected_rng = rng_;
    Engine projected(scenario_, recipe_, projected_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    if (!remove_one(projected.state_.hand, Card::FieldBlower)) return false;
    projected.state_.discard.push_back(Card::FieldBlower);
    projected.state_.path_lock_removed = true;

    return projected.secret_box_full_combo_axes_visible() &&
        projected.secret_box_cost_plan().has_value();
  }

  bool issue_3057_preserve_forest_stadium_action() const {
    return issue_3064_direct_forest_route_available() ||
        issue_3064_deferred_forest_route_available();
  }

  int hand_count_issue_3057(const Card card) const {
    // part_014c currently asks Chaotic Swell, then Field Blower, then runs the
    // Pineco planner. Hide both competing lock-removal cards for a held-Forest
    // route so Forest itself replaces Path. For the K1 deferred route, hide only
    // Chaotic Swell so Field Blower clears Path and the fixed-point Item loop can
    // search Forest with Secret Box before retrying the Forretress planner.
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Late-turn ordering: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if ((card == Card::ChaoticSwell || card == Card::FieldBlower) &&
        issue_3064_direct_forest_route_available()) {
      return 0;
    }
    if (card == Card::ChaoticSwell &&
        issue_3064_deferred_forest_route_available()) {
      return 0;
    }
    return hand_count(card);
  }
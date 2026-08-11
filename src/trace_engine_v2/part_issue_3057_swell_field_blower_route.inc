  bool issue_3057_preserve_forest_stadium_action() const {
    // Field Blower can discard the modeled Path to the Peak as an Item, while
    // Forest of Vitality needs the turn's single Stadium play to let a current-turn
    // Pineco evolve into Forretress ex after the first turn. When that complete
    // public route is live, preserve Chaotic Swell and spend Field Blower first:
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Stadium / Item / evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Pineco architecture: https://github.com/FlareZ123/pokemon-sims/issues/1118
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    if (!rule_box_abilities_locked() || item_locked() || state_.stadium_used ||
        state_.turn <= 1 || hand_count(Card::FieldBlower) == 0 ||
        hand_count(Card::ChaoticSwell) == 0 ||
        hand_count(Card::ForestOfVitality) == 0 ||
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

  int hand_count_issue_3057(const Card card) const {
    // part_014c's existing order is Chaotic Swell -> Field Blower -> Pineco plan.
    // Hide only Chaotic Swell from that late-turn layer when the verified Forest
    // route above is live. Field Blower then removes Path without consuming the
    // Stadium action, and every other hand-count query remains unchanged:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // https://github.com/FlareZ123/pokemon-sims/issues/3057
    if (card == Card::ChaoticSwell &&
        issue_3057_preserve_forest_stadium_action()) {
      return 0;
    }
    return hand_count(card);
  }

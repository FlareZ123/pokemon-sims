  bool issue_3057_live_forest_entry_timing() const {
    if (state_.stadium_used || state_.turn <= 1 ||
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

    // Forest of Vitality changes the normal evolution timer only for a Grass
    // Pokemon that entered play this turn, and never during the player's first
    // turn. A prior-turn Pineco therefore does not justify reserving the Stadium.
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Core Stadium and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Earliest-route/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    return forretress_available &&
        (current_turn_pineco || pineco_can_enter_this_turn);
  }

  bool issue_3057_live_forest_entry_route() const {
    // Path to the Peak suppresses Forretress ex's Rule Box Ability until Path is
    // replaced or discarded. This pre-removal predicate is used only to choose the
    // connector that should remove Path without consuming the Forest Stadium play.
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    return rule_box_abilities_locked() && issue_3057_live_forest_entry_timing();
  }

  bool issue_3057_direct_forest_over_path() const {
    // A held Forest is itself the dominant Path-removal connector. Playing it
    // replaces Path, consumes the Stadium play once, remains active, and supplies
    // the same-turn Grass evolution permission. Field Blower and Chaotic Swell
    // add no setup axis in this state and should remain available as resources.
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Stadium replacement procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Connector domination/resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    return issue_3057_live_forest_entry_route() &&
        hand_count(Card::ForestOfVitality) > 0;
  }

  bool issue_3057_deferred_forest_via_field_blower() const {
    if (!issue_3057_live_forest_entry_route() || item_locked() ||
        hand_count(Card::ForestOfVitality) > 0 ||
        hand_count(Card::FieldBlower) == 0 ||
        hand_count(Card::SecretBox) == 0 || !prizes_known() ||
        deck_count_after_search_started(Card::ForestOfVitality) == 0) {
      return false;
    }

    // Project the exact public state after Field Blower. Secret Box may justify
    // preserving the Stadium play only when K1 proves Forest is in deck and the
    // existing AMR/DCI-aware planner proves both the complete route and three legal
    // discard costs from that post-removal state.
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Item, Stadium, search, and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1 and connector/resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Secret Box ALS implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1118_secret_box.inc
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    if (!remove_one(projected.state_.hand, Card::FieldBlower)) return false;
    projected.state_.discard.push_back(Card::FieldBlower);
    projected.state_.path_lock_removed = true;

    return projected.secret_box_full_combo_axes_visible() &&
        projected.secret_box_cost_plan().has_value();
  }

  bool issue_3057_post_blower_secret_box_forest() const {
    if (rule_box_abilities_locked() || state_.stadium_used || state_.turn <= 1 ||
        !forretress_combo_enabled() || !has_any_regi() || !need_energy() ||
        hand_count(Card::ForestOfVitality) > 0 ||
        hand_count(Card::SecretBox) == 0 || !prizes_known() ||
        deck_count_after_search_started(Card::ForestOfVitality) == 0 ||
        !issue_3057_live_forest_entry_timing()) {
      return false;
    }

    // This is the exact post-Field-Blower continuation of the pre-removal K1
    // projection. Re-check the current public state and DCI plan before spending
    // the ACE SPEC so unrelated Item loops cannot inherit the special route.
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // K1/DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    return secret_box_full_combo_axes_visible() &&
        secret_box_cost_plan().has_value();
  }

  bool issue_3057_play_direct_forest_over_path() {
    if (!issue_3057_direct_forest_over_path() ||
        !remove_one(state_.hand, Card::ForestOfVitality)) {
      return false;
    }

    // Playing the different Stadium discards the modeled Path, restores Rule Box
    // Abilities, and leaves Forest active. This wrapper performs only that Stadium
    // action; the existing Forretress planner retains control of evolution and
    // Exploding Energy afterwards.
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Stadium replacement/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    state_.stadium = Stadium::ForestOfVitality;
    state_.stadium_used = true;
    state_.path_lock_removed = true;
    trace("PLAY STADIUM", "R-FOREST-VITALITY-01; R-PATH-01; P-AXIS-01",
          "Played Forest of Vitality directly over the modeled Path to the Peak; "
          "the live current-turn Pineco evolution route retained Forest in play.");
    return true;
  }

  bool play_chaotic_swell_issue_3057() {
    if (issue_3057_direct_forest_over_path()) {
      return issue_3057_play_direct_forest_over_path();
    }
    if (issue_3057_deferred_forest_via_field_blower()) {
      // Preserve the one Stadium play for the K1-proven Secret Box -> Forest leg.
      // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
      // Secret Box / Forest: https://api.pokemontcg.io/v2/cards/sv6-163 https://api.pokemontcg.io/v2/cards/me1-117
      // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
      return false;
    }
    return play_chaotic_swell();
  }

  bool play_field_blower_issue_3057() {
    if (issue_3057_direct_forest_over_path()) {
      // Forest already owns the dominant Path-removal action, so preserve the Item.
      // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
      // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
      // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
      return false;
    }

    const bool deferred_forest = issue_3057_deferred_forest_via_field_blower();
    const bool played = play_field_blower();
    if (!played || !deferred_forest) return played;

    // The K1 projection above already proves the post-Blower Secret Box route and
    // its three AMR/DCI costs. Resolve that connector immediately while the Stadium
    // action remains unused, then re-enter the ordinary Forretress fixed point so
    // the searched Forest can enable the current-turn Pineco evolution.
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Item/search/Stadium/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Dynamic DCI and connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    if (play_secret_box()) {
      advance_forretress_combo();
    }
    return true;
  }

  void play_items_until_stable_issue_3057(const bool allow_flexible_discard) {
    // The historical generic turn path reaches this point immediately after its
    // ordinary Field Blower pass. Resolve the same proven connector before generic
    // Items can consume its three DCI costs, then continue the existing Item loop.
    // Secret Box / Forest / Field Blower: https://api.pokemontcg.io/v2/cards/sv6-163 https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
    if (issue_3057_post_blower_secret_box_forest() && play_secret_box()) {
      advance_forretress_combo();
    }
    play_items_until_stable(allow_flexible_discard);
  }

  int hand_count_issue_3057(const Card card) const {
    if (issue_3057_direct_forest_over_path() &&
        (card == Card::ChaoticSwell || card == Card::FieldBlower)) {
      // Hide only the generic pre-Forretress removal actions. The ordinary
      // advance_forretress_combo() call then plays Forest directly over Path.
      // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
      // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
      // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
      // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
      return 0;
    }
    if (card == Card::ChaoticSwell &&
        issue_3057_deferred_forest_via_field_blower()) {
      // Forest is known in deck and the post-Blower Secret Box route is payable,
      // so preserve the Stadium action while leaving Field Blower visible.
      // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
      // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
      // Secret Box / Forest: https://api.pokemontcg.io/v2/cards/sv6-163 https://api.pokemontcg.io/v2/cards/me1-117
      // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
      return 0;
    }
    return hand_count(card);
  }

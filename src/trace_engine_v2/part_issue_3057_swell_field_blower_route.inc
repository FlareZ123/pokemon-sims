  bool issue_3064_live_current_turn_forest_route() const {
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

    // Forest of Vitality permits a Grass Pokemon to evolve into another Grass
    // Pokemon during its entry turn after the player's first turn. Pineco and
    // Forretress ex are Grass, and Exploding Energy can then attach Basic Grass
    // Energy before Forretress ex Knocks itself Out:
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Evolution and Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Pineco architecture: https://github.com/FlareZ123/pokemon-sims/issues/1118
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return forretress_available &&
        (current_turn_pineco || pineco_can_enter_this_turn);
  }

  bool issue_3064_direct_forest_over_path_route() const {
    // A held Forest is itself the direct Stadium replacement for Path to the Peak.
    // It clears the Stadium-based Rule Box Ability lock, stays in play, and supplies
    // the same-turn Grass evolution permission. Spending Field Blower first would
    // consume an extra Item connector with no additional setup progress:
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return rule_box_abilities_locked() &&
        issue_3064_live_current_turn_forest_route() &&
        hand_count(Card::ForestOfVitality) > 0;
  }

  bool issue_3064_deferred_prized_forest_via_tapu_route() const {
    if (!rule_box_abilities_locked() || item_locked() ||
        !issue_3064_live_current_turn_forest_route() ||
        hand_count(Card::ForestOfVitality) > 0 ||
        hand_count(Card::FieldBlower) == 0 || !supporter_allowed() ||
        !prizes_known() ||
        prize_count_after_reveal(Card::ForestOfVitality) == 0 ||
        hand_count(Card::TapuLeleGX) == 0 || in_play(Card::TapuLeleGX) ||
        deck_count_after_search_started(Card::Gladion) == 0) {
      return false;
    }

    const bool current_turn_pineco =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn == state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      return pokemon.card == Card::Pineco &&
                          pokemon.entered_turn == state_.turn;
                    });
    const int required_open_bench_slots = current_turn_pineco ? 1 : 2;

    // This is a fully observable K1 bridge. Path suppresses Tapu Lele-GX's Wonder
    // Tag, so Field Blower first restores that Rule Box Ability without spending the
    // Stadium action. Wonder Tag can then search Gladion; Gladion exchanges itself
    // for the known prized Forest; Forest finally replaces the still-open Stadium
    // slot and enables the current-turn Pineco evolution:
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Bench, Ability, Supporter, Prize, search, and Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return bench_space() >= required_open_bench_slots;
  }

  bool issue_3064_resolve_deferred_prized_forest_via_tapu() {
    if (!state_.path_lock_removed || state_.stadium_used || item_locked() ||
        hand_count(Card::ForestOfVitality) > 0 || !supporter_allowed() ||
        !prizes_known() ||
        prize_count_after_reveal(Card::ForestOfVitality) == 0 ||
        hand_count(Card::TapuLeleGX) == 0 || in_play(Card::TapuLeleGX) ||
        bench_space() <= 0 || deck_count_after_search_started(Card::Gladion) == 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX)) {
      return false;
    }

    // Wonder Tag triggers only when Tapu Lele-GX is played from hand to the Bench.
    // The preceding Field Blower has removed Path, so the Rule Box Ability is live.
    // Search only the proved K1 Gladion target for this focused bridge:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Core Bench, Ability, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (!bench_from_hand(Card::TapuLeleGX, false)) {
      throw std::logic_error("#3064 Tapu Lele-GX bridge lost its Bench target");
    }
    record_deck_search_knowledge("Wonder Tag");
    if (!move_deck_to_hand(Card::Gladion)) {
      throw std::logic_error("#3064 Wonder Tag Gladion target disappeared");
    }
    shuffle(state_.deck);
    trace("WONDER TAG", "R-GAME-ABILITY; P-KNOWLEDGE-01",
          "Wonder Tag searched Gladion for the known prized Forest of Vitality route.");

    // Gladion looks at all Prize cards, chooses one, exchanges it with Gladion,
    // and shuffles the Prize cards. K1 already proves Forest is the setup-advancing
    // target, so this branch never consults an unrevealed Prize identity:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Prize and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (!remove_one(state_.hand, Card::Gladion)) {
      throw std::logic_error("#3064 Gladion bridge lost its Supporter");
    }
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto forest = std::find(state_.prizes.begin(), state_.prizes.end(),
                                  Card::ForestOfVitality);
    if (forest == state_.prizes.end()) {
      throw std::logic_error("#3064 known prized Forest target disappeared");
    }
    *forest = Card::Gladion;
    state_.hand.push_back(Card::ForestOfVitality);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01",
          "Gladion exchanged itself for the known prized Forest of Vitality.");
    return true;
  }

  void play_basics_from_hand_issue_3064() {
    // part_014c reaches this point immediately after its first Path-removal actions.
    // If the confirmed K1 Field Blower bridge is now live, finish Wonder Tag and
    // Gladion before the ordinary Basic loop, then reuse the existing Forretress
    // planner to play Forest and continue the exact Pineco line:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo_legacy.inc
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (issue_3064_resolve_deferred_prized_forest_via_tapu()) {
      advance_forretress_combo();
    }
    play_basics_from_hand();
  }

  int hand_count_issue_3057(const Card card) const {
    // Direct held Forest dominates both generic lock-removal cards. The deferred
    // K1 Tapu -> Gladion route suppresses only Chaotic Swell so Field Blower can
    // clear Path while preserving the one Stadium action for Forest:
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (issue_3064_direct_forest_over_path_route() &&
        (card == Card::ChaoticSwell || card == Card::FieldBlower)) {
      return 0;
    }
    if (card == Card::ChaoticSwell &&
        issue_3064_deferred_prized_forest_via_tapu_route()) {
      return 0;
    }
    return hand_count(card);
  }
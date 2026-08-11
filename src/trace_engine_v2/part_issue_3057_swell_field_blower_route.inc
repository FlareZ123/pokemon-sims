  bool issue_3064_live_current_turn_forest_combo() const {
    // Forest of Vitality matters here only for same-turn Grass evolution. A prior-turn
    // Pineco can already evolve normally, so Forest would not advance that axis:
    // Forest / Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Evolution and Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (state_.stadium_used || state_.turn <= 1 || !forretress_combo_enabled() ||
        !has_any_regi() || !need_energy() || !might_be_unseen(Card::Grass)) return false;

    const auto prior = [this](const Pokemon& p) {
      return p.card == Card::Pineco && p.entered_turn < state_.turn;
    };
    if ((state_.active && prior(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), prior)) return false;

    const bool current =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn == state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), [this](const Pokemon& p) {
          return p.card == Card::Pineco && p.entered_turn == state_.turn;
        });
    const bool can_enter = in_play_count(Card::Pineco) == 0 && bench_space() > 0 &&
        (hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco));
    const bool forretress = hand_count(Card::ForretressEx) > 0 || might_be_unseen(Card::ForretressEx);
    return forretress && (current || can_enter);
  }

  bool issue_3064_direct_forest_over_path_available() const {
    // A held Forest replaces Path and grants the same-turn evolution effect in one
    // Stadium action, so Field Blower or Chaotic Swell first is connector-dominated:
    // Forest / Path / Field Blower / Swell: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sm12-187
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    return rule_box_abilities_locked() && hand_count(Card::ForestOfVitality) > 0 &&
        issue_3064_live_current_turn_forest_combo();
  }

  bool issue_3064_deferred_forest_via_legacy_star_available() const {
    // With Forest public in discard, Field Blower can clear Path without spending the
    // Stadium action; restored Legacy Star can then recover Forest deterministically:
    // Regidrago VSTAR / Path / Field Blower / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/me1-117
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return rule_box_abilities_locked() && !item_locked() &&
        hand_count(Card::FieldBlower) > 0 && hand_count(Card::ForestOfVitality) == 0 &&
        count_of(state_.discard, Card::ForestOfVitality) > 0 && !state_.vstar_power_used &&
        has_vstar() && issue_3064_live_current_turn_forest_combo();
  }

  bool issue_3064_should_recover_forest_with_legacy_star() const {
    // This predicate is evaluated after the non-Stadium Path removal has resolved:
    // Regidrago VSTAR / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/me1-117
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return state_.path_lock_removed && !rule_box_abilities_locked() &&
        hand_count(Card::ForestOfVitality) == 0 &&
        count_of(state_.discard, Card::ForestOfVitality) > 0 && !state_.vstar_power_used &&
        has_vstar() && ability_available_for_pokemon(Card::RegidragoVstar) &&
        issue_3064_live_current_turn_forest_combo();
  }

  bool use_legacy_star_issue3064() {
    if (!issue_3064_should_recover_forest_with_legacy_star()) {
      // All unrelated Legacy Star states preserve the historical implementation:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013_legacy_star_override.inc
      return use_legacy_star_before3064();
    }

    // Forest was public before the random seven-card discard, so route selection
    // does not depend on hidden top-deck identities:
    // Legacy Star / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/me1-117
    // Hidden information: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    state_.vstar_power_used = true;
    outcome_.used_legacy_star = true;
    std::vector<Card> discarded;
    for (int i = 0; i < 7 && !state_.deck.empty(); ++i) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      state_.discard.push_back(top);
      state_.discarded_this_turn.push_back(top);
      discarded.push_back(top);
    }

    std::vector<Card> recovered;
    if (!recover_discard_to_hand(Card::ForestOfVitality))
      throw std::logic_error("Legacy Star Forest target disappeared");
    recovered.push_back(Card::ForestOfVitality);

    const auto recover_second = [this, &recovered](const Card card) {
      if (recovered.size() >= 2U || count_of(state_.discard, card) == 0) return false;
      if (!recover_discard_to_hand(card))
        throw std::logic_error("Legacy Star secondary target disappeared");
      recovered.push_back(card);
      return true;
    };

    // Exploding Energy completes Energy, so the optional second recovery serves only
    // another still-missing payload or mobility axis:
    // Forretress ex / Blender / Burnet / Latias ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/sv8-76
    // Existing priority: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013_legacy_star_override.inc
    if (need_payload() && !item_locked() && payload_might_be_in_deck())
      recover_second(Card::BrilliantBlender);
    if (recovered.size() < 2U && need_payload() && supporter_allowed() && payload_might_be_in_deck())
      recover_second(Card::ProfessorBurnet);
    if (recovered.size() < 2U && need_active_vstar() && state_.active &&
        is_basic(state_.active->card) && bench_space() > 0 && !in_play(Card::LatiasEx) &&
        ability_available_for_pokemon(Card::LatiasEx)) recover_second(Card::LatiasEx);

    trace("LEGACY STAR", "R-RVS-01; R-VSTAR-01; R-FOREST-VITALITY-01",
          "Discarded top 7: " + join_cards(discarded) +
              "; recovered deferred Forest route: " + join_cards(recovered) + ".");

    // Reuse the existing planner for Forest, evolution, and Exploding Energy:
    // Forest / Forretress ex: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Planner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo_legacy.inc
    advance_forretress_combo();
    return true;
  }

  bool use_legacy_star() {
    // The historical late run_turn calls this exact base symbol. Intercept only the
    // confirmed post-Field-Blower public-Forest state and delegate every other state:
    // Legacy Star / Field Blower / Path / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/me1-117
    // Late call site: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return issue_3064_should_recover_forest_with_legacy_star()
        ? use_legacy_star_issue3064()
        : use_legacy_star_before3064();
  }

  bool issue_3057_preserve_forest_stadium_action() const {
    // Compatibility predicate for focused regression coverage:
    // https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    return issue_3064_direct_forest_over_path_available() ||
        issue_3064_deferred_forest_via_legacy_star_available();
  }

  int hand_count_issue_3057(const Card card) const {
    // Late order is Swell -> Blower -> Forest planner. Direct Forest hides both
    // generic unlocks; deferred Legacy Star hides only Swell so Blower remains:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // Forest / Path / Blower / Swell: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sm12-187
    if ((card == Card::ChaoticSwell || card == Card::FieldBlower) &&
        issue_3064_direct_forest_over_path_available()) return 0;
    if (card == Card::ChaoticSwell && issue_3064_deferred_forest_via_legacy_star_available()) return 0;
    return hand_count(card);
  }

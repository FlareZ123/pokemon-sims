  bool issue_3064_live_current_turn_forest_combo() const {
    // Forest of Vitality is valuable here only for its exceptional same-turn Grass
    // evolution permission. A prior-turn Pineco can already evolve normally, so
    // spending the Stadium action on Forest would not advance this setup axis:
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Evolution and Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined governing bug: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective bug: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (state_.stadium_used || state_.turn <= 1 ||
        !forretress_combo_enabled() || !has_any_regi() || !need_energy() ||
        !might_be_unseen(Card::Grass)) {
      return false;
    }

    const auto prior_turn_pineco = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::Pineco && pokemon.entered_turn < state_.turn;
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
    return forretress_available && (current_turn_pineco || pineco_can_enter_this_turn);
  }

  bool issue_3064_direct_forest_over_path_available() const {
    // A Stadium played from hand replaces the Stadium already in play. When Forest
    // is already held, it removes Path and supplies the same-turn evolution permission
    // with one action; spending Field Blower first is connector-dominated:
    // Forest / Path / Field Blower: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125
    // Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return rule_box_abilities_locked() && hand_count(Card::ForestOfVitality) > 0 &&
        issue_3064_live_current_turn_forest_combo();
  }

  bool issue_3064_deferred_forest_via_legacy_star_available() const {
    // Path suppresses Legacy Star. A public-discard Forest creates a deterministic
    // Field Blower -> Legacy Star -> Forest route that leaves the Stadium action unused:
    // Regidrago VSTAR / Path / Field Blower / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/me1-117
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return rule_box_abilities_locked() && !item_locked() &&
        hand_count(Card::FieldBlower) > 0 && hand_count(Card::ForestOfVitality) == 0 &&
        count_of(state_.discard, Card::ForestOfVitality) > 0 &&
        !state_.vstar_power_used && has_vstar() && issue_3064_live_current_turn_forest_combo();
  }

  bool issue_3064_should_recover_forest_with_legacy_star() const {
    // After Field Blower removes Path, require the exact public state before spending
    // the once-per-game VSTAR Power:
    // Regidrago VSTAR / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/me1-117
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    return state_.path_lock_removed && !rule_box_abilities_locked() &&
        hand_count(Card::ForestOfVitality) == 0 &&
        count_of(state_.discard, Card::ForestOfVitality) > 0 &&
        !state_.vstar_power_used && has_vstar() &&
        ability_available_for_pokemon(Card::RegidragoVstar) &&
        issue_3064_live_current_turn_forest_combo();
  }

  bool use_legacy_star_issue3064() {
    if (!issue_3064_should_recover_forest_with_legacy_star()) {
      // Historical #1417 implementation retained under an explicit composition name:
      // https://github.com/FlareZ123/pokemon-sims/issues/1417
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
      return use_legacy_star_issue1417_original();
    }

    // Forest was public before Legacy Star's random top-seven discard, so choosing
    // this route does not branch on hidden identities:
    // Legacy Star / Forest: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/me1-117
    // Hidden information: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    state_.vstar_power_used = true;
    outcome_.used_legacy_star = true;
    std::vector<Card> discarded;
    for (int card = 0; card < 7 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      state_.discard.push_back(top);
      state_.discarded_this_turn.push_back(top);
      discarded.push_back(top);
    }

    std::vector<Card> recovered;
    if (!recover_discard_to_hand(Card::ForestOfVitality)) {
      throw std::logic_error("Legacy Star Forest target disappeared");
    }
    recovered.push_back(Card::ForestOfVitality);

    const auto recover_second_if = [this, &recovered](const Card card) {
      if (recovered.size() >= 2U || count_of(state_.discard, card) == 0) return false;
      if (!recover_discard_to_hand(card)) {
        throw std::logic_error("Legacy Star secondary target disappeared");
      }
      recovered.push_back(card);
      return true;
    };

    // Exploding Energy completes the Energy axis, so use the optional second Legacy
    // Star selection only for a separate still-missing payload or mobility axis:
    // Forretress ex / Blender / Burnet / Latias ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://api.pokemontcg.io/v2/cards/sv8-76
    // Existing recovery priority: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013_legacy_star_override.inc
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (need_payload() && !item_locked() && payload_might_be_in_deck()) {
      recover_second_if(Card::BrilliantBlender);
    }
    if (recovered.size() < 2U && need_payload() && supporter_allowed() &&
        payload_might_be_in_deck()) {
      recover_second_if(Card::ProfessorBurnet);
    }
    if (recovered.size() < 2U && need_active_vstar() && state_.active &&
        is_basic(state_.active->card) && bench_space() > 0 && !in_play(Card::LatiasEx) &&
        ability_available_for_pokemon(Card::LatiasEx)) {
      recover_second_if(Card::LatiasEx);
    }

    trace("LEGACY STAR", "R-RVS-01; R-VSTAR-01; R-FOREST-VITALITY-01",
          "Discarded top 7: " + join_cards(discarded) +
              "; recovered deferred Forest route: " + join_cards(recovered) + ".");

    // Forest is now held and the Stadium action remains unused; reuse the existing
    // Forretress planner for Forest, evolution, and Exploding Energy:
    // Forest / Forretress ex: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Planner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo_legacy.inc
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    advance_forretress_combo();
    return true;
  }

  bool use_legacy_star_issue_1417() {
    // #1152 and later layers call this historical symbol. Intercept only the exact
    // post-Field-Blower public-discard Forest state and otherwise delegate unchanged:
    // Legacy Star / Forest / Field Blower / Path: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/swsh6-148
    // #1152 bridge: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c_issue_1152_bridge.inc
    // Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if (issue_3064_should_recover_forest_with_legacy_star()) {
      return use_legacy_star_issue3064();
    }
    return use_legacy_star_issue1417_original();
  }

  bool issue_3057_preserve_forest_stadium_action() const {
    // Compatibility predicate for focused #3057/#3064 regressions:
    // https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
    // https://github.com/FlareZ123/pokemon-sims/issues/3064
    return issue_3064_direct_forest_over_path_available() ||
        issue_3064_deferred_forest_via_legacy_star_available();
  }

  int hand_count_issue_3057(const Card card) const {
    // Late order is Swell -> Blower -> Forest planner. Direct Forest dominance hides
    // both generic unlocks; deferred Legacy Star hides only Swell so Blower remains:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // Forest / Path / Field Blower / Chaotic Swell: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sm12-187
    // Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
    if ((card == Card::ChaoticSwell || card == Card::FieldBlower) &&
        issue_3064_direct_forest_over_path_available()) {
      return 0;
    }
    if (card == Card::ChaoticSwell && issue_3064_deferred_forest_via_legacy_star_available()) {
      return 0;
    }
    return hand_count(card);
  }

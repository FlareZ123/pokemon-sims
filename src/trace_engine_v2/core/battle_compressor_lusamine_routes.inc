  int issue_3545_lusamine_public_horizon_score(
      Engine projected, const bool permit_payload,
      bool* const rule_box_locked_out = nullptr) const {
    // Compare any Lusamine package against the complete existing #3545 Item policy,
    // then continue through every configured public turn using the same deterministic
    // pre-/post-Supporter actions. Unknown start-of-turn draws remain omitted by the
    // established projection adapter:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    projected.play_items_until_stable_issue3545_refined_base(permit_payload);
    for (;;) {
      projected.issue_3545_prepare_supporter_projection();
      projected.issue_3545_choose_supporter_from_prepared_projection();
      projected.issue_3545_continue_after_supporter(projected);
      projected.resolve_powerglass_end_turn();
      if (projected.state_.turn >= projected.scenario_.max_turn) break;
      issue_3545_advance_public_state_one_turn(projected);
    }
    if (rule_box_locked_out != nullptr) {
      *rule_box_locked_out = projected.rule_box_abilities_locked();
    }
    return projected.issue_3545_setup_route_score();
  }

  bool issue_3545_project_bc_vs_lusamine(
      Engine& projected, const std::vector<Card>& targets) const {
    if (!projected.issue_3545_hidden_project_compressor(projected, targets) ||
        !remove_one(projected.state_.hand, Card::VsSeeker)) {
      return false;
    }
    projected.state_.discard.push_back(Card::VsSeeker);
    return projected.recover_discard_to_hand(Card::Lusamine);
  }

  int issue_3545_lusamine_item_lock_bank_score(
      const bool permit_payload) const {
    // On the Supporter-legal turn immediately before a persistent Item lock, one
    // VS Seeker can recover Lusamine after BC stages Lusamine + Crispin + Burnet.
    // Lusamine then recovers both future Supporters into hand before the Item lock,
    // preserving the sole VS while banking deterministic Energy and payload axes.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Item / Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Persistent Item-lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (state_.turn_ended || item_locked() || !supporter_allowed() ||
        uses_full_supporter_lock(scenario_.locks) ||
        hand_count(Card::BattleCompressor) == 0 ||
        hand_count(Card::VsSeeker) != 1 || state_.turn + 2 > scenario_.max_turn ||
        !item_locked_on_turn(state_.turn + 1) || !need_energy() || !need_payload() ||
        hand_count(Card::Lusamine) > 0 || count_of(state_.discard, Card::Lusamine) > 0 ||
        hand_count(Card::Crispin) > 0 || count_of(state_.discard, Card::Crispin) > 0 ||
        hand_count(Card::ProfessorBurnet) > 0 ||
        count_of(state_.discard, Card::ProfessorBurnet) > 0 ||
        count_of(state_.deck, Card::Lusamine) == 0 ||
        count_of(state_.deck, Card::Crispin) == 0 ||
        count_of(state_.deck, Card::ProfessorBurnet) == 0) {
      return -1;
    }

    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    const std::vector<Card> targets{
        Card::Lusamine, Card::Crispin, Card::ProfessorBurnet};
    if (!issue_3545_project_bc_vs_lusamine(projected, targets)) return -1;

    projected.play_items_until_stable_issue3545_refined_base(permit_payload);
    const int crispin_hand_before = projected.hand_count(Card::Crispin);
    const int burnet_hand_before = projected.hand_count(Card::ProfessorBurnet);
    const int crispin_discard_before = count_of(projected.state_.discard, Card::Crispin);
    const int burnet_discard_before =
        count_of(projected.state_.discard, Card::ProfessorBurnet);
    if (!projected.issue_3545_recovered_supporter_is_used(
            projected, Card::Lusamine)) {
      return -1;
    }
    const bool crispin_banked =
        projected.hand_count(Card::Crispin) > crispin_hand_before &&
        count_of(projected.state_.discard, Card::Crispin) < crispin_discard_before;
    const bool burnet_banked =
        projected.hand_count(Card::ProfessorBurnet) > burnet_hand_before &&
        count_of(projected.state_.discard, Card::ProfessorBurnet) < burnet_discard_before;
    if (!crispin_banked || !burnet_banked) return -1;

    projected.issue_3545_continue_after_supporter(projected);
    projected.resolve_powerglass_end_turn();

    bool crispin_used = false;
    bool burnet_used = false;
    while (projected.state_.turn < projected.scenario_.max_turn) {
      issue_3545_advance_public_state_one_turn(projected);
      projected.issue_3545_prepare_supporter_projection();
      const int crispin_before_use = projected.hand_count(Card::Crispin);
      const int burnet_before_use = projected.hand_count(Card::ProfessorBurnet);
      projected.issue_3545_choose_supporter_from_prepared_projection();
      crispin_used = crispin_used ||
          (crispin_before_use > 0 &&
           projected.hand_count(Card::Crispin) < crispin_before_use);
      burnet_used = burnet_used ||
          (burnet_before_use > 0 &&
           projected.hand_count(Card::ProfessorBurnet) < burnet_before_use);
      if (!projected.state_.supporter_used) return -1;
      projected.issue_3545_continue_after_supporter(projected);
      projected.resolve_powerglass_end_turn();
    }

    // Both staged cards must be real future Supporter actions. This keeps the three-
    // card BC package from earning credit for a redundant second Supporter slot.
    if (!crispin_used || !burnet_used) return -1;
    return projected.issue_3545_setup_route_score();
  }

  std::vector<Card> issue_3545_lusamine_item_lock_bank_targets(
      const bool permit_payload) const {
    const int route_score = issue_3545_lusamine_item_lock_bank_score(permit_payload);
    if (route_score < 0) return {};
    std::mt19937_64 baseline_rng = rng_;
    bool ignored_lock = false;
    const int baseline_score = issue_3545_lusamine_public_horizon_score(
        issue_3545_projection(baseline_rng), permit_payload, &ignored_lock);
    if (route_score <= baseline_score) return {};
    return {Card::Lusamine, Card::Crispin, Card::ProfessorBurnet};
  }

  int issue_3545_lusamine_swell_unlock_score(
      const std::vector<Card>& targets, const bool permit_payload,
      bool* const unlocked_out = nullptr) const {
    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    if (!issue_3545_project_bc_vs_lusamine(projected, targets)) return -1;

    projected.play_items_until_stable_issue3545_refined_base(permit_payload);
    const int swell_hand_before = projected.hand_count(Card::ChaoticSwell);
    const int swell_discard_before =
        count_of(projected.state_.discard, Card::ChaoticSwell);
    if (!projected.issue_3545_recovered_supporter_is_used(
            projected, Card::Lusamine)) {
      return -1;
    }
    const bool swell_recovered =
        projected.hand_count(Card::ChaoticSwell) > swell_hand_before &&
        count_of(projected.state_.discard, Card::ChaoticSwell) < swell_discard_before;
    if (!swell_recovered || !projected.play_chaotic_swell() ||
        projected.rule_box_abilities_locked()) {
      return -1;
    }
    if (unlocked_out != nullptr) *unlocked_out = true;

    // Production run_turn() replays Chaotic Swell immediately after the Supporter.
    // The ordinary #3545 continuation omits that Stadium stage, so model only this
    // deterministic public action here; do not resolve Legacy Star or inspect its
    // random cards.
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Production ordering: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    projected.issue_3545_continue_after_supporter(projected);
    projected.resolve_powerglass_end_turn();
    while (projected.state_.turn < projected.scenario_.max_turn) {
      issue_3545_advance_public_state_one_turn(projected);
      projected.issue_3545_prepare_supporter_projection();
      projected.issue_3545_choose_supporter_from_prepared_projection();
      projected.issue_3545_continue_after_supporter(projected);
      projected.resolve_powerglass_end_turn();
    }
    return projected.issue_3545_setup_route_score();
  }

  std::vector<Card> issue_3545_lusamine_swell_unlock_targets(
      const bool permit_payload) const {
    if (state_.turn_ended || item_locked() || !supporter_allowed() ||
        uses_full_supporter_lock(scenario_.locks) ||
        hand_count(Card::BattleCompressor) == 0 ||
        hand_count(Card::VsSeeker) != 1 || !rule_box_abilities_locked() ||
        hand_count(Card::Lusamine) > 0 || count_of(state_.discard, Card::Lusamine) > 0 ||
        count_of(state_.deck, Card::Lusamine) == 0) {
      return {};
    }

    std::mt19937_64 baseline_rng = rng_;
    bool baseline_locked = true;
    const int baseline_score = issue_3545_lusamine_public_horizon_score(
        issue_3545_projection(baseline_rng), permit_payload, &baseline_locked);

    std::vector<Card> best;
    int best_score = baseline_score;
    for (const Card supporter : {Card::ProfessorBurnet, Card::Crispin, Card::Arven}) {
      if (count_of(state_.deck, supporter) == 0 || hand_count(supporter) > 0) continue;
      std::vector<Card> targets{Card::Lusamine, supporter};
      if (count_of(state_.discard, Card::ChaoticSwell) == 0) {
        if (count_of(state_.deck, Card::ChaoticSwell) == 0) continue;
        targets.push_back(Card::ChaoticSwell);
      }
      bool unlocked = false;
      int score = issue_3545_lusamine_swell_unlock_score(
          targets, permit_payload, &unlocked);
      if (score < 0 || !unlocked) continue;
      // Clearing a public Rule Box lock is a strict resource improvement even when
      // the coarse setup-axis score ties. Use one point only as a connector-dominance
      // tie-break; it cannot outrank any scored setup axis.
      if (baseline_locked) ++score;
      if (score > best_score) {
        best_score = score;
        best = std::move(targets);
      }
    }
    return best;
  }

  std::vector<Card> issue_3545_lusamine_route_targets(
      const bool permit_payload) const {
    std::vector<Card> best =
        issue_3545_lusamine_item_lock_bank_targets(permit_payload);
    const std::vector<Card> swell =
        issue_3545_lusamine_swell_unlock_targets(permit_payload);
    if (swell.empty()) return best;
    if (best.empty()) return swell;

    const int bank_score = issue_3545_lusamine_item_lock_bank_score(permit_payload);
    bool unlocked = false;
    const int swell_score =
        issue_3545_lusamine_swell_unlock_score(swell, permit_payload, &unlocked) +
        (unlocked ? 1 : 0);
    return swell_score > bank_score ? swell : best;
  }

  bool issue_3545_play_bc_vs_lusamine_package(
      const std::vector<Card>& targets) {
    if (targets.empty() || targets.size() > 3U || item_locked() ||
        hand_count(Card::BattleCompressor) == 0 || hand_count(Card::VsSeeker) == 0) {
      return false;
    }
    if (!remove_one(state_.hand, Card::BattleCompressor)) {
      throw std::logic_error("Lusamine-route Battle Compressor disappeared");
    }
    state_.discard.push_back(Card::BattleCompressor);
    record_deck_search_knowledge("Battle Compressor");

    std::vector<Card> discarded;
    for (const Card target : targets) {
      if (!remove_one(state_.deck, target)) {
        throw std::logic_error("Lusamine-route Battle Compressor target disappeared");
      }
      state_.discard.push_back(target);
      state_.discarded_this_turn.push_back(target);
      discarded.push_back(target);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-BATTLE-COMPRESSOR-01; R-GAME-ITEM; P-JIT-01; P-DCI-01; P-KNOWLEDGE-01",
          "Searched and discarded Lusamine-route targets: " +
              join_cards(discarded) + ".");

    if (!remove_one(state_.hand, Card::VsSeeker)) {
      throw std::logic_error("Lusamine-route VS Seeker disappeared");
    }
    state_.discard.push_back(Card::VsSeeker);
    if (!recover_discard_to_hand(Card::Lusamine)) {
      throw std::logic_error("Lusamine-route recovery target disappeared");
    }
    trace("PLAY ITEM", "R-VS-SEEKER-01; R-GAME-ITEM; P-COMPRESS-01",
          "Recovered Lusamine from the discard pile for the proven BC/VS route.");
    return true;
  }

  void play_items_until_stable_issue3545(const bool permit_payload) {
    if (state_.turn_ended) return;
    const std::vector<Card> targets =
        issue_3545_lusamine_route_targets(permit_payload);
    if (!targets.empty()) {
      issue_3545_play_bc_vs_lusamine_package(targets);
    }
    play_items_until_stable_issue3545_refined_base(permit_payload);
  }

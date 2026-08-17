  int issue_3545_setup_route_score() const {
    int score = 0;
    if (!need_regi()) score += 2000;
    if (!need_vstar()) score += 2000;
    if (!need_energy()) score += 2500;
    if (!need_payload()) score += 3000;
    if (!need_active_vstar()) score += 2000;
    if (active_is_vstar()) score += 500;
    if (const Pokemon* target = target_regi()) {
      score += 100 * apex_energy_progress(*target);
    }
    if (!setup_axis_missing()) score += 100000;
    return score;
  }

  Engine issue_3545_projection(std::mt19937_64& shadow_rng) const {
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    return projected;
  }

  void issue_3545_advance_public_state_one_turn(Engine& projected) const {
    // Future-bank decisions deliberately omit the unknown start-of-turn draw. They
    // may rely only on cards already visible now while applying the exact current
    // public turn reset. This prevents a future-card oracle and keeps projections
    // synchronized with Dark Asset and transient-lock lifecycle state.
    // Turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Future-card policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Lifecycle owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/turn_lifecycle.inc
    projected.reset_per_turn_state(projected.state_.turn + 1);
  }

  std::optional<Card> issue_3545_first_payload_in_deck() const {
    // Preserve the established attack-payload priority already used by Brilliant
    // Blender. Battle Compressor can choose any cards from deck, so only physically
    // present Dragon payloads are eligible.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Blender policy owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010_blender_thinning_override.inc
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX,
                               Card::Appletun}) {
      if (count_of(state_.deck, payload) > 0) return payload;
    }
    return std::nullopt;
  }

  bool issue_3545_recovered_supporter_is_used(Engine& projected,
                                               const Card candidate) const {
    const int recovered_count = projected.hand_count(candidate);
    projected.choose_supporter();
    return projected.state_.supporter_used &&
           projected.hand_count(candidate) < recovered_count;
  }

  bool issue_3545_payload_outlet_dominates_compressor() const {
    const bool payload_already_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (!payload_already_held) return false;

    // When a visible mandatory-discard connector can spend the held Dragon while
    // advancing another missing axis, it dominates using BC solely for payload.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (deck_search_available()) {
      for (const Card item : {Card::MysteriousTreasure, Card::QuickBall,
                              Card::EarthenVessel}) {
        if (hand_count(item) == 0) continue;
        const auto cost = choose_discard(true, true, true, item);
        if (cost && is_payload(*cost)) return true;
      }
    }
    if (hand_count(Card::UltraBall) > 0 && ultra_ball_has_legal_target()) {
      Engine simulated = *this;
      const auto first = simulated.choose_discard(true, true, true, Card::UltraBall);
      if (first && is_payload(*first) && remove_one(simulated.state_.hand, *first) &&
          simulated.choose_discard(false, true, true, Card::UltraBall).has_value()) {
        return true;
      }
    }
    return supporter_allowed() && hand_count(Card::Serena) > 0;
  }

  bool issue_3545_project_direct_compressor_payload() {
    if (item_locked() || hand_count(Card::BattleCompressor) == 0 ||
        !need_payload() || !can_play_payload_this_turn() ||
        issue_3545_payload_outlet_dominates_compressor()) {
      return false;
    }
    const auto payload = issue_3545_first_payload_in_deck();
    if (!payload || !remove_one(state_.hand, Card::BattleCompressor) ||
        !remove_one(state_.deck, *payload)) {
      return false;
    }
    state_.discard.push_back(Card::BattleCompressor);
    state_.discard.push_back(*payload);
    state_.discarded_this_turn.push_back(*payload);
    deck_seen_ = true;
    return true;
  }

  int issue_3545_direct_compressor_baseline_score(const bool evaluate_next_turn) const {
    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);

    // Compare Supporter-via-VS routes with the cheaper direct BC payload route. This
    // prevents redundant lines such as spending VS Seeker on Professor Burnet when
    // BC itself already supplies the only missing Dragon payload.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    projected.issue_3545_project_direct_compressor_payload();
    if (evaluate_next_turn) {
      issue_3545_advance_public_state_one_turn(projected);
    }
    projected.choose_supporter();
    return projected.issue_3545_setup_route_score();
  }

  std::optional<Card> issue_3545_vs_seeker_target() const {
    if (state_.turn_ended || item_locked() || hand_count(Card::VsSeeker) == 0 ||
        uses_full_supporter_lock(scenario_.locks)) {
      return std::nullopt;
    }

    const bool immediate_supporter = supporter_allowed();
    const bool bank_before_scheduled_item_lock =
        !immediate_supporter && state_.turn < scenario_.max_turn &&
        item_locked_on_turn(state_.turn + 1) &&
        (state_.supporter_used || first_turn_going_first());
    if (!immediate_supporter && !bank_before_scheduled_item_lock) {
      return std::nullopt;
    }

    // VS Seeker puts one Supporter from discard into hand. Evaluate each recovery
    // against both the ordinary Supporter line and any cheaper direct BC payload
    // line already available in hand. When BC and VS genuinely combine, the
    // projection resolves the BC payload before applying the recovered Supporter.
    // This accepts Crispin+BC Energy/payload synergy while rejecting redundant
    // Burnet+BC payload duplication.
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Persistent T2 Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    std::mt19937_64 baseline_rng = rng_;
    Engine baseline = issue_3545_projection(baseline_rng);
    if (bank_before_scheduled_item_lock) {
      issue_3545_advance_public_state_one_turn(baseline);
    }
    baseline.choose_supporter();
    int best_score = baseline.issue_3545_setup_route_score();
    best_score = std::max(
        best_score,
        issue_3545_direct_compressor_baseline_score(bank_before_scheduled_item_lock));

    std::optional<Card> best;
    std::vector<Card> seen;
    for (const Card candidate : state_.discard) {
      if (!is_supporter(candidate) ||
          std::find(seen.begin(), seen.end(), candidate) != seen.end()) {
        continue;
      }
      seen.push_back(candidate);

      std::mt19937_64 shadow_rng = rng_;
      Engine projected = issue_3545_projection(shadow_rng);
      if (!remove_one(projected.state_.hand, Card::VsSeeker)) continue;
      projected.state_.discard.push_back(Card::VsSeeker);
      if (!projected.recover_discard_to_hand(candidate)) continue;

      // If BC is also held, test the legal combined line before Supporter choice.
      // The helper declines BC when a better held-payload connector already exists.
      projected.issue_3545_project_direct_compressor_payload();
      if (bank_before_scheduled_item_lock) {
        issue_3545_advance_public_state_one_turn(projected);
      }
      if (!issue_3545_recovered_supporter_is_used(projected, candidate)) continue;

      const int score = projected.issue_3545_setup_route_score();
      if (score > best_score) {
        best_score = score;
        best = candidate;
      }
    }
    return best;
  }

  bool play_vs_seeker() {
    const std::optional<Card> target = issue_3545_vs_seeker_target();
    if (!target) return false;

    // VS Seeker resolves as an Item and is discarded after moving exactly one
    // Supporter from discard to hand. The recovered Supporter is still governed by
    // the ordinary one-Supporter-per-turn scheduler.
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Item / Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
    if (!remove_one(state_.hand, Card::VsSeeker)) {
      throw std::logic_error("projected VS Seeker disappeared");
    }
    state_.discard.push_back(Card::VsSeeker);
    if (!recover_discard_to_hand(*target)) {
      throw std::logic_error("projected VS Seeker Supporter target disappeared");
    }
    trace("PLAY ITEM", "R-VS-SEEKER-01; R-GAME-ITEM; P-COMPRESS-01",
          "Recovered " + std::string(name(*target)) + " from the discard pile.");
    return true;
  }

  std::optional<Card> issue_3545_compressor_supporter_target() const {
    if (state_.turn_ended || item_locked() || hand_count(Card::BattleCompressor) == 0 ||
        hand_count(Card::VsSeeker) == 0 || uses_full_supporter_lock(scenario_.locks)) {
      return std::nullopt;
    }

    const bool immediate_supporter = supporter_allowed();
    const bool bank_before_scheduled_item_lock =
        !immediate_supporter && state_.turn < scenario_.max_turn &&
        item_locked_on_turn(state_.turn + 1) &&
        (state_.supporter_used || first_turn_going_first());
    if (!immediate_supporter && !bank_before_scheduled_item_lock) {
      return std::nullopt;
    }

    // BC may turn a deck Supporter into VS Seeker access. Evaluate the combined
    // route against both the ordinary Supporter line and a direct BC payload line.
    // The projected BC resolution also includes the one Dragon payload that the
    // real selector will take when payload is live, allowing true synergy such as
    // BC(payload + Crispin) -> VS Seeker -> Crispin for simultaneous payload/Energy.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // JIT / connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    std::mt19937_64 baseline_rng = rng_;
    Engine baseline = issue_3545_projection(baseline_rng);
    if (bank_before_scheduled_item_lock) {
      issue_3545_advance_public_state_one_turn(baseline);
    }
    baseline.choose_supporter();
    int best_score = baseline.issue_3545_setup_route_score();
    best_score = std::max(
        best_score,
        issue_3545_direct_compressor_baseline_score(bank_before_scheduled_item_lock));

    std::optional<Card> best;
    std::vector<Card> seen;
    for (const Card candidate : state_.deck) {
      if (!is_supporter(candidate) ||
          std::find(seen.begin(), seen.end(), candidate) != seen.end()) {
        continue;
      }
      seen.push_back(candidate);

      std::mt19937_64 shadow_rng = rng_;
      Engine projected = issue_3545_projection(shadow_rng);
      projected.deck_seen_ = true;
      if (!remove_one(projected.state_.hand, Card::BattleCompressor)) continue;
      projected.state_.discard.push_back(Card::BattleCompressor);
      if (!remove_one(projected.state_.deck, candidate)) continue;
      projected.state_.discard.push_back(candidate);
      projected.state_.discarded_this_turn.push_back(candidate);

      if (projected.need_payload() && projected.can_play_payload_this_turn()) {
        if (const auto payload = projected.issue_3545_first_payload_in_deck()) {
          if (remove_one(projected.state_.deck, *payload)) {
            projected.state_.discard.push_back(*payload);
            projected.state_.discarded_this_turn.push_back(*payload);
          }
        }
      }

      if (!remove_one(projected.state_.hand, Card::VsSeeker)) continue;
      projected.state_.discard.push_back(Card::VsSeeker);
      if (!projected.recover_discard_to_hand(candidate)) continue;
      if (bank_before_scheduled_item_lock) {
        issue_3545_advance_public_state_one_turn(projected);
      }
      if (!issue_3545_recovered_supporter_is_used(projected, candidate)) continue;

      const int score = projected.issue_3545_setup_route_score();
      if (score > best_score) {
        best_score = score;
        best = candidate;
      }
    }
    return best;
  }

  std::vector<Card> issue_3545_battle_compressor_targets() const {
    std::vector<Card> targets;
    if (state_.turn_ended || item_locked() || hand_count(Card::BattleCompressor) == 0 ||
        state_.deck.empty()) {
      return targets;
    }

    const auto supporter = issue_3545_compressor_supporter_target();
    if (supporter) targets.push_back(*supporter);

    const bool payload_live = need_payload() && can_play_payload_this_turn();
    const bool payload_is_dominated =
        !supporter.has_value() && issue_3545_payload_outlet_dominates_compressor();
    if (payload_live && !payload_is_dominated) {
      // StrictJit and MatchupFlexJit require same-ready-turn payload timing, so one
      // live Dragon is sufficient and preserves later attack options. The explicit
      // NoDiscardControl profile may use remaining BC capacity for broader payload
      // compression because its model permits early discard.
      // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      const std::size_t payload_limit =
          strict_payload_timing() ? std::min<std::size_t>(3U, targets.size() + 1U) : 3U;
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX,
                                 Card::Appletun}) {
        if (targets.size() >= payload_limit || targets.size() >= 3U) break;
        if (count_of(state_.deck, payload) > count_of(targets, payload)) {
          targets.push_back(payload);
        }
      }
    }

    // BC says "up to 3". Only NoDiscardControl may spend otherwise-unused slots
    // on stable low-impact thinning, and only when a visible same-turn draw refresh
    // can actually realize that compression. Other profiles stop early.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    const bool refresh_available = scenario_.dci == DciProfile::NoDiscardControl &&
        supporter_allowed() && !supporter.has_value() &&
        (hand_count(Card::TateLiza) > 0 || hand_count(Card::Serena) > 0);
    if (refresh_available) {
      for (const Card card : {Card::PathToPeak, Card::ErikasInvitation,
                              Card::Guzma, Card::Channeler,
                              Card::FieldBlower, Card::ChaoticSwell}) {
        if (targets.size() >= 3U) break;
        if (count_of(state_.deck, card) > count_of(targets, card)) {
          targets.push_back(card);
        }
      }
    }
    return targets;
  }

  bool play_battle_compressor() {
    const std::vector<Card> targets = issue_3545_battle_compressor_targets();
    if (targets.empty()) return false;

    // Resolve the printed effect exactly: search the deck for up to three cards,
    // discard the selected cards, then shuffle. The legal deck inspection establishes
    // K1. Current-turn Dragon discards are recorded for strict-JIT provenance.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
    if (!remove_one(state_.hand, Card::BattleCompressor)) {
      throw std::logic_error("projected Battle Compressor disappeared");
    }
    state_.discard.push_back(Card::BattleCompressor);
    record_deck_search_knowledge("Battle Compressor");

    std::vector<Card> discarded;
    for (const Card target : targets) {
      const auto found = std::find(state_.deck.begin(), state_.deck.end(), target);
      if (found == state_.deck.end()) {
        throw std::logic_error("projected Battle Compressor deck target disappeared");
      }
      state_.deck.erase(found);
      state_.discard.push_back(target);
      state_.discarded_this_turn.push_back(target);
      discarded.push_back(target);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-BATTLE-COMPRESSOR-01; R-GAME-ITEM; P-JIT-01; P-DCI-01; P-KNOWLEDGE-01",
          "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

  void play_items_until_stable_issue3545(const bool permit_payload) {
    if (state_.turn_ended) return;
    for (int guard = 0; guard < 8; ++guard) {
      bool issue_3545_action = false;

      // VS may recover an already-discarded Supporter before BC is considered, but
      // its target projection already compares that route with direct BC. BC then
      // precedes the legacy Item fixed point so a legal deck-discard route can
      // preserve the one-copy ACE SPEC. Recheck VS after BC because BC may have
      // deliberately placed the selected Supporter into discard.
      // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
      // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
      // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      if (play_vs_seeker()) issue_3545_action = true;
      if (play_battle_compressor()) issue_3545_action = true;
      if (play_vs_seeker()) issue_3545_action = true;

      play_items_until_stable_issue_3057(permit_payload);

      if (play_vs_seeker()) issue_3545_action = true;
      if (play_battle_compressor()) issue_3545_action = true;
      if (play_vs_seeker()) issue_3545_action = true;
      if (!issue_3545_action) break;
    }
  }

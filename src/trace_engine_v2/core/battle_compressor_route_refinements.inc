  int issue_3545_next_public_turn_score(
      const std::vector<Card>& targets,
      const std::optional<Card> required_current_supporter = std::nullopt) const {
    if (targets.empty()) return -1;
    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    if (!issue_3545_hidden_project_compressor(projected, targets)) return -1;

    projected.play_vs_seeker();
    const int required_before = required_current_supporter
        ? projected.hand_count(*required_current_supporter)
        : 0;
    projected.choose_supporter_issue3545_projection();
    if (required_current_supporter &&
        (!projected.state_.supporter_used ||
         projected.hand_count(*required_current_supporter) >= required_before)) {
      return -1;
    }
    projected.issue_3545_continue_after_supporter(projected);
    projected.resolve_powerglass_end_turn();
    if (projected.state_.turn >= projected.scenario_.max_turn) {
      return projected.issue_3545_setup_route_score();
    }
    issue_3545_advance_public_state_one_turn(projected);
    projected.choose_supporter_issue3545_projection();
    projected.issue_3545_continue_after_supporter(projected);
    return projected.issue_3545_setup_route_score();
  }

  int issue_3545_hold_compressor_for_next_public_turn_score() const {
    if (!strict_payload_timing() || state_.turn_ended || item_locked() ||
        hand_count(Card::BattleCompressor) != 1 || state_.turn >= scenario_.max_turn ||
        item_locked_on_turn(state_.turn + 1) || !need_payload() ||
        can_play_payload_this_turn() || !deck_seen_) {
      return -1;
    }

    // Once K1 has established that a Dragon payload physically remains in deck, the
    // last Battle Compressor itself is a deterministic future same-ready-turn outlet.
    // Evaluate the current public turn with that Item hidden from every discard/search
    // policy, advance one turn without the unknown draw, then spend BC only after the
    // ordinary public setup sequence has made its payload timing legal.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // K1 / future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    if (!std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) return -1;

    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    if (!remove_one(projected.state_.hand, Card::BattleCompressor)) return -1;

    projected.issue_3545_prepare_supporter_projection();
    projected.issue_3545_choose_supporter_from_prepared_projection();
    projected.issue_3545_continue_after_supporter(projected);
    projected.resolve_powerglass_end_turn();
    projected.state_.hand.push_back(Card::BattleCompressor);

    if (projected.state_.turn >= projected.scenario_.max_turn) {
      return -1;
    }
    issue_3545_advance_public_state_one_turn(projected);

    // Keep the reserved BC invisible while ordinary next-turn setup runs. This
    // prevents DCI costs from consuming the exact outlet being evaluated.
    if (!remove_one(projected.state_.hand, Card::BattleCompressor)) return -1;
    projected.issue_3545_prepare_supporter_projection();
    projected.issue_3545_choose_supporter_from_prepared_projection();
    projected.issue_3545_continue_after_supporter(projected);
    projected.state_.hand.push_back(Card::BattleCompressor);

    projected.issue_3545_project_direct_compressor_payload();
    return projected.issue_3545_setup_route_score();
  }

  bool issue_3545_should_reserve_last_compressor_for_ready_turn() const {
    const int hold_score = issue_3545_hold_compressor_for_next_public_turn_score();
    if (hold_score < 0) return false;

    const std::vector<Card> immediate = issue_3545_refined_battle_compressor_targets();
    if (immediate.empty()) return true;
    const int immediate_score = issue_3545_next_public_turn_score(immediate);
    return hold_score > immediate_score;
  }

  bool issue_3545_grant_stage_available(
      const std::vector<Card>& existing_targets) const {
    // Grant has no setup-axis Supporter value in these goldfish recipes, while its
    // discard-pile auxiliary effect is an independent action: discard two other
    // cards from hand to return Grant to hand. The existing record_ready() path uses
    // that effect only after Active VSTAR + GGF are complete and payload is the sole
    // missing axis. BC may stage Grant before that turn if the current public hand
    // already proves a held Dragon plus an ordinary second recovery cost.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Existing Grant recovery owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    if (!strict_payload_timing() || state_.turn_ended || item_locked() ||
        !need_payload() || can_play_payload_this_turn() ||
        state_.turn >= scenario_.max_turn || existing_targets.size() >= 3U ||
        count_of(state_.deck, Card::Grant) <= count_of(existing_targets, Card::Grant) ||
        std::find(existing_targets.begin(), existing_targets.end(),
                  Card::StevensResolve) != existing_targets.end()) {
      return false;
    }

    const auto payload_it =
        std::find_if(state_.hand.begin(), state_.hand.end(), is_payload);
    if (payload_it == state_.hand.end()) return false;

    Engine probe = *this;
    remove_one(probe.state_.hand, Card::BattleCompressor);
    if (!remove_one(probe.state_.hand, *payload_it)) return false;
    probe.state_.hand.erase(
        std::remove(probe.state_.hand.begin(), probe.state_.hand.end(), Card::Grant),
        probe.state_.hand.end());
    const bool flex_fodder = scenario_.dci == DciProfile::MatchupFlexJit;
    const auto ordinary = probe.choose_discard(false, flex_fodder);
    return ordinary.has_value() && *ordinary != Card::Grant;
  }

  bool issue_3545_celestial_roar_density_realized(
      const std::vector<Card>& targets) const {
    if (targets.empty() || targets.size() > 3U || state_.turn_ended || item_locked()) {
      return false;
    }

    const int energy_after_bc =
        static_cast<int>(std::count_if(state_.deck.begin(), state_.deck.end(), is_energy)) -
        static_cast<int>(std::count_if(targets.begin(), targets.end(), is_energy));
    const int deck_after_bc = static_cast<int>(state_.deck.size() - targets.size());
    if (energy_after_bc <= 0 || deck_after_bc <= 3) return false;

    // Whether the production Celestial Roar policy elects to attack is decided from
    // public state before its random top-three resolution. Celestial Roar attaches
    // every Energy card it finds, including modeled Double Dragon Energy. Use a fixed
    // private shadow RNG only to execute the copied attack after that admission
    // decision; no drawn identity is inspected by BC. The existing 64-seed target-
    // invariance regression independently protects this boundary.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    std::mt19937_64 shadow_rng{3545135ULL};
    Engine projected = issue_3545_projection(shadow_rng);
    if (!issue_3545_hidden_project_compressor(projected, targets)) return false;
    projected.issue_3545_prepare_supporter_projection();
    projected.issue_3545_choose_supporter_from_prepared_projection();
    projected.issue_3545_continue_after_supporter(projected);
    if (projected.state_.turn_ended) return false;
    return projected.use_celestial_roar();
  }

  std::vector<Card> issue_3545_add_safe_celestial_roar_thinning(
      std::vector<Card> targets) const {
    if (targets.size() >= 3U) return targets;

    // Removing a non-Energy card before a live Celestial Roar strictly increases the
    // Energy density of the shuffled deck. Restrict strict/flex candidates to cards
    // already stable dead in the modeled state so probability improvement never
    // authorizes sacrificing a live connector, lock answer, Supporter, or Dragon.
    // Wishful Baton is unused by the goldfish setup model; Dipplin has no Applin line;
    // Battle VIP Pass is dead after turn one; excess Regidrago copies are dead only
    // after their represented axis is already covered.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    const std::array<std::pair<Card, bool>, 5> safe_candidates{{
        {Card::Dipplin, true},
        {Card::WishfulBaton, true},
        {Card::BattleVipPass, state_.turn > 1},
        {Card::RegidragoV, in_play_count(Card::RegidragoV) >= 2},
        {Card::RegidragoVstar, has_vstar()},
    }};

    for (const auto& [candidate, stable_dead] : safe_candidates) {
      if (!stable_dead || targets.size() >= 3U ||
          count_of(state_.deck, candidate) <= count_of(targets, candidate)) {
        continue;
      }
      std::vector<Card> trial = targets;
      trial.push_back(candidate);
      if (issue_3545_celestial_roar_density_realized(trial)) {
        targets = std::move(trial);
      }
    }
    return targets;
  }

  std::vector<Card> issue_3545_refined_battle_compressor_targets() const {
    std::vector<Card> best = issue_3545_hidden_battle_compressor_targets();
    int best_score = issue_3545_next_public_turn_score(best);
    const auto powerglass_energy = issue_3545_powerglass_energy_target();

    // Strict and matchup-flex can use BC + VS as a deterministic two-turn package:
    // BC stages Steven plus a Basic Energy, Powerglass recovers that Energy at end of
    // T1, and Steven reserves VSTAR + Crispin + Mysterious Treasure. On T2 Crispin
    // finishes GGF and Treasure discards the held Dragon on the actual ready turn.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    if (strict_payload_timing() && powerglass_energy &&
        hand_count(Card::VsSeeker) > 0 && supporter_allowed() &&
        count_of(state_.deck, Card::StevensResolve) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      const std::vector<Card> steven_targets{Card::StevensResolve,
                                             *powerglass_energy};
      const int steven_score = issue_3545_next_public_turn_score(
          steven_targets, Card::StevensResolve);
      if (steven_score > best_score) {
        best_score = steven_score;
        best = steven_targets;
      }
    }

    if (issue_3545_grant_stage_available(best)) {
      best.push_back(Card::Grant);
    }

    // No-control needs one early Dragon bank. A deterministic Powerglass Energy
    // improvement outranks a second or third early payload with no additional setup
    // axis value.
    if (scenario_.dci == DciProfile::NoDiscardControl && powerglass_energy &&
        count_of(best, *powerglass_energy) == 0) {
      std::vector<Card> trial = best;
      bool changed = false;
      if (trial.size() < 3U) {
        trial.push_back(*powerglass_energy);
        changed = true;
      } else {
        bool kept_first_payload = false;
        for (std::size_t i = 0; i < trial.size(); ++i) {
          if (!is_payload(trial[i])) continue;
          if (!kept_first_payload) {
            kept_first_payload = true;
            continue;
          }
          trial[i] = *powerglass_energy;
          changed = true;
          break;
        }
      }
      if (changed) {
        const int trial_score = issue_3545_next_public_turn_score(trial);
        if (trial_score > best_score) {
          best_score = trial_score;
          best = std::move(trial);
        }
      }
    }

    return issue_3545_add_safe_celestial_roar_thinning(std::move(best));
  }

  bool play_battle_compressor_refined_routes() {
    const std::vector<Card> hidden_targets =
        issue_3545_hidden_battle_compressor_targets();
    const std::vector<Card> targets = issue_3545_refined_battle_compressor_targets();
    if (targets.empty() || targets == hidden_targets) {
      return play_battle_compressor_hidden_routes_issue3545_base();
    }

    if (!remove_one(state_.hand, Card::BattleCompressor)) {
      throw std::logic_error("refined-route Battle Compressor disappeared");
    }
    state_.discard.push_back(Card::BattleCompressor);
    record_deck_search_knowledge("Battle Compressor");

    std::vector<Card> discarded;
    for (const Card target : targets) {
      if (!remove_one(state_.deck, target)) {
        throw std::logic_error("refined-route Battle Compressor target disappeared");
      }
      state_.discard.push_back(target);
      state_.discarded_this_turn.push_back(target);
      discarded.push_back(target);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-BATTLE-COMPRESSOR-01; R-GAME-ITEM; P-JIT-01; P-DCI-01; P-KNOWLEDGE-01",
          "Searched and discarded refined-route targets: " + join_cards(discarded) + ".");
    return true;
  }

  void play_items_until_stable_issue3545(const bool permit_payload) {
    if (state_.turn_ended) return;
    for (int guard = 0; guard < 8; ++guard) {
      if (issue_3545_should_reserve_last_compressor_for_ready_turn()) {
        // Hide the reserved BC from the entire legacy fixed point, including discard
        // costs. Restore it afterward and re-evaluate because ordinary search, Energy,
        // or evolution actions may have moved the state into the legal payload window.
        if (!remove_one(state_.hand, Card::BattleCompressor)) {
          throw std::logic_error("reserved Battle Compressor disappeared");
        }
        play_items_until_stable_issue3545_base(permit_payload);
        state_.hand.push_back(Card::BattleCompressor);
        if (issue_3545_should_reserve_last_compressor_for_ready_turn()) break;
        continue;
      }

      const bool refined_action = play_battle_compressor_refined_routes();
      play_items_until_stable_issue3545_base(permit_payload);
      if (!refined_action || state_.turn_ended) break;
    }
  }

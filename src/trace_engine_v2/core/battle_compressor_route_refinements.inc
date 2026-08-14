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

    // This is a future-ready-turn proof, so reserve the currently held Dragon
    // symbolically rather than calling choose_discard(true), which correctly protects
    // that Dragon under strict JIT on the current pre-ready turn. The ordinary second
    // Grant cost is still validated by the shared DCI selector after removing the BC
    // that will be consumed now and excluding Grant itself from legal costs.
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
    return best;
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
      const bool refined_action = play_battle_compressor_refined_routes();
      play_items_until_stable_issue3545_base(permit_payload);
      if (!refined_action || state_.turn_ended) break;
    }
  }

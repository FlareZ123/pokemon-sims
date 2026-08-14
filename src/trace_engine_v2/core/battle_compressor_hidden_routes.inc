  bool issue_3545_hidden_stochastic_supporter(const Card card) const {
    // Serena and Tate & Liza can expose random future draws. Hidden-route projections
    // must not choose a BC target from the outcome of an unseen future card, so the
    // deterministic bank/recovery refinements below leave those Supporters on the
    // already-validated base policy.
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    return card == Card::Serena || card == Card::TateLiza;
  }

  bool issue_3545_hidden_project_compressor(
      Engine& projected, const std::vector<Card>& targets) const {
    if (targets.empty() || targets.size() > 3U ||
        !remove_one(projected.state_.hand, Card::BattleCompressor)) {
      return false;
    }
    projected.state_.discard.push_back(Card::BattleCompressor);
    projected.deck_seen_ = true;
    for (const Card target : targets) {
      if (!remove_one(projected.state_.deck, target)) return false;
      projected.state_.discard.push_back(target);
      projected.state_.discarded_this_turn.push_back(target);
    }
    return true;
  }

  int issue_3545_hidden_current_score(
      const std::vector<Card>& targets,
      const std::optional<Card> required_supporter = std::nullopt,
      const bool resolve_powerglass = false) const {
    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    if (!targets.empty() && !issue_3545_hidden_project_compressor(projected, targets)) {
      return -1;
    }

    // BC resolves inside the Item fixed point. Reproduce the deterministic public
    // continuation available before and after the one Supporter play without calling
    // the #3545 wrapper recursively.
    projected.play_vs_seeker();
    projected.play_items_until_stable_issue_3057(true);
    projected.play_vs_seeker();

    int required_before = 0;
    if (required_supporter) required_before = projected.hand_count(*required_supporter);
    projected.choose_supporter();
    if (required_supporter &&
        (!projected.state_.supporter_used ||
         projected.hand_count(*required_supporter) >= required_before)) {
      return -1;
    }
    projected.issue_3545_continue_after_supporter(projected);
    if (resolve_powerglass && !projected.state_.turn_ended) {
      projected.resolve_powerglass_end_turn();
    }
    return projected.issue_3545_setup_route_score();
  }

  int issue_3545_hidden_bank_score(const std::vector<Card>& targets,
                                   const Card current_supporter) const {
    if (issue_3545_hidden_stochastic_supporter(current_supporter)) return -1;

    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    if (!issue_3545_hidden_project_compressor(projected, targets)) return -1;

    // First VS must recover the Supporter BC deliberately staged for the current
    // Supporter play. Resolve it explicitly so an older discard Supporter cannot
    // displace the route under evaluation.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    if (!remove_one(projected.state_.hand, Card::VsSeeker)) return -1;
    projected.state_.discard.push_back(Card::VsSeeker);
    if (!projected.recover_discard_to_hand(current_supporter)) return -1;

    const int current_before = projected.hand_count(current_supporter);
    projected.choose_supporter();
    if (!projected.state_.supporter_used ||
        projected.hand_count(current_supporter) >= current_before) {
      return -1;
    }
    projected.issue_3545_continue_after_supporter(projected);
    if (projected.state_.turn_ended) return -1;

    // The played Supporter has returned to discard. A second VS Seeker can recover
    // that same physical Supporter and bank it in hand before the next-turn Item lock.
    // A second Supporter only belongs in BC's three-card discard when it causes the
    // ordinary VS bank selector to choose a strictly stronger future route.
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Persistent Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    if (!projected.play_vs_seeker()) return -1;

    projected.resolve_powerglass_end_turn();
    issue_3545_advance_public_state_one_turn(projected);
    projected.choose_supporter();
    projected.issue_3545_continue_after_supporter(projected);
    return projected.issue_3545_setup_route_score();
  }

  std::vector<Card> issue_3545_hidden_bank_targets(
      const std::vector<Card>& base_targets) const {
    const auto current_supporter = issue_3545_compressor_supporter_target();
    if (!current_supporter || issue_3545_hidden_stochastic_supporter(*current_supporter) ||
        hand_count(Card::VsSeeker) < 2 || state_.turn >= scenario_.max_turn ||
        !item_locked_on_turn(state_.turn + 1)) {
      return base_targets;
    }

    std::vector<Card> reduced;
    bool kept_current = false;
    for (const Card target : base_targets) {
      if (!is_supporter(target)) {
        reduced.push_back(target);
        continue;
      }
      if (!kept_current && target == *current_supporter) {
        reduced.push_back(target);
        kept_current = true;
      }
    }
    if (!kept_current) reduced.insert(reduced.begin(), *current_supporter);
    if (reduced.size() > 3U) reduced.resize(3U);

    int best_score = issue_3545_hidden_bank_score(reduced, *current_supporter);
    std::vector<Card> best = reduced;

    // Compare any different future Supporter that physically remains in deck. This
    // preserves a two-Supporter BC discard only when the second VS bank makes the
    // next public turn strictly better than reusing the just-played Supporter.
    std::vector<Card> seen;
    for (const Card candidate : state_.deck) {
      if (!is_supporter(candidate) || candidate == *current_supporter ||
          std::find(seen.begin(), seen.end(), candidate) != seen.end()) {
        continue;
      }
      seen.push_back(candidate);
      if (best.size() >= 3U) break;
      std::vector<Card> trial = reduced;
      trial.push_back(candidate);
      const int score = issue_3545_hidden_bank_score(trial, *current_supporter);
      if (score > best_score) {
        best_score = score;
        best = std::move(trial);
      }
    }

    const int base_score = issue_3545_hidden_bank_score(base_targets, *current_supporter);
    if (base_score > best_score) return base_targets;
    if (best_score < 0) return base_targets;
    return best;
  }

  std::vector<Card> issue_3545_hidden_klara_targets() const {
    // Klara can move up to two Pokémon and up to two Basic Energy from discard into
    // hand. BC can therefore become a deterministic deck-to-hand connector for a
    // missing Regidrago, evolution, mobility Basic, or manual Basic Energy when Klara
    // is already the legal Supporter action. Evaluate only exact recoverable card
    // classes and require the existing Supporter chooser to actually consume Klara.
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    if (!supporter_allowed() || hand_count(Card::Klara) == 0 ||
        hand_count(Card::BattleCompressor) == 0) {
      return {};
    }

    const std::array<Card, 8> candidates{
        Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
        Card::Oricorio, Card::TapuLeleGX, Card::CrobatV,
        Card::Grass, Card::Fire};

    std::vector<Card> selected;
    int best_score = issue_3545_hidden_current_score({}, std::nullopt, false);
    for (int slot = 0; slot < 3; ++slot) {
      std::optional<Card> best_card;
      int best_trial_score = best_score;
      for (const Card candidate : candidates) {
        if (count_of(state_.deck, candidate) <= count_of(selected, candidate)) continue;
        std::vector<Card> trial = selected;
        trial.push_back(candidate);
        const int score = issue_3545_hidden_current_score(
            trial, Card::Klara, false);
        if (score > best_trial_score) {
          best_trial_score = score;
          best_card = candidate;
        }
      }
      if (!best_card) break;
      selected.push_back(*best_card);
      best_score = best_trial_score;
    }
    return selected;
  }

  std::vector<Card> issue_3545_hidden_powerglass_prune(
      const std::vector<Card>& targets) const {
    const bool has_supporter =
        std::any_of(targets.begin(), targets.end(), is_supporter);
    if (!has_supporter) return targets;

    const auto current_supporter = issue_3545_compressor_supporter_target();
    if (!current_supporter || issue_3545_hidden_stochastic_supporter(*current_supporter)) {
      return targets;
    }

    std::vector<Card> reduced;
    bool removed_basic_energy = false;
    for (const Card target : targets) {
      if (!removed_basic_energy && (target == Card::Grass || target == Card::Fire)) {
        removed_basic_energy = true;
        continue;
      }
      reduced.push_back(target);
    }
    if (!removed_basic_energy || reduced.empty()) return targets;

    const int full_score = issue_3545_hidden_current_score(
        targets, *current_supporter, true);
    const int reduced_score = issue_3545_hidden_current_score(
        reduced, *current_supporter, true);
    return reduced_score >= full_score && reduced_score >= 0 ? reduced : targets;
  }

  std::vector<Card> issue_3545_hidden_battle_compressor_targets() const {
    const std::vector<Card> base_targets = issue_3545_battle_compressor_targets();
    std::vector<Card> best = issue_3545_hidden_bank_targets(base_targets);
    best = issue_3545_hidden_powerglass_prune(best);

    const std::vector<Card> klara = issue_3545_hidden_klara_targets();
    if (!klara.empty()) {
      const int klara_score = issue_3545_hidden_current_score(
          klara, Card::Klara, false);
      const int current_score = best.empty()
          ? issue_3545_hidden_current_score({}, std::nullopt, false)
          : issue_3545_hidden_current_score(best, std::nullopt, false);
      if (klara_score > current_score) best = klara;
    }
    return best;
  }

  bool play_battle_compressor_hidden_routes() {
    const std::vector<Card> base_targets = issue_3545_battle_compressor_targets();
    const std::vector<Card> targets = issue_3545_hidden_battle_compressor_targets();
    if (targets.empty() || targets == base_targets) return false;

    if (!remove_one(state_.hand, Card::BattleCompressor)) {
      throw std::logic_error("hidden-route Battle Compressor disappeared");
    }
    state_.discard.push_back(Card::BattleCompressor);
    record_deck_search_knowledge("Battle Compressor");

    std::vector<Card> discarded;
    for (const Card target : targets) {
      if (!remove_one(state_.deck, target)) {
        throw std::logic_error("hidden-route Battle Compressor target disappeared");
      }
      state_.discard.push_back(target);
      state_.discarded_this_turn.push_back(target);
      discarded.push_back(target);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-BATTLE-COMPRESSOR-01; R-GAME-ITEM; P-JIT-01; P-DCI-01; P-KNOWLEDGE-01",
          "Searched and discarded hidden-route targets: " + join_cards(discarded) + ".");
    return true;
  }

  void play_items_until_stable_issue3545(const bool permit_payload) {
    if (state_.turn_ended) return;
    for (int guard = 0; guard < 8; ++guard) {
      const bool hidden_action = play_battle_compressor_hidden_routes();
      play_items_until_stable_issue3545_base(permit_payload);
      if (!hidden_action || state_.turn_ended) break;
    }
  }

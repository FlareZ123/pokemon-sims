  bool issue_3545_future_bank_supporter_is_used(
      const std::vector<Card>& targets, const Card current_supporter,
      const Card future_supporter) const {
    if (targets.empty() || current_supporter == future_supporter ||
        !is_supporter(current_supporter) || !is_supporter(future_supporter)) {
      return false;
    }

    // A distinct second Supporter is a real two-VS bank only if the first VS
    // explicitly recovers the current Supporter, that Supporter is actually played,
    // the second VS explicitly recovers the staged future Supporter, and that exact
    // future card is the Supporter action on the public next turn. Generic next-turn
    // setup credit is insufficient because another held Supporter could otherwise
    // create the score while the BC-staged card is merely discard-cost fodder.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Supporter / Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Persistent Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Connector domination: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    std::mt19937_64 shadow_rng = rng_;
    Engine projected = issue_3545_projection(shadow_rng);
    if (!issue_3545_hidden_project_compressor(projected, targets) ||
        !remove_one(projected.state_.hand, Card::VsSeeker)) {
      return false;
    }
    projected.state_.discard.push_back(Card::VsSeeker);
    if (!projected.recover_discard_to_hand(current_supporter)) return false;

    const int current_before = projected.hand_count(current_supporter);
    projected.choose_supporter_issue3545_projection();
    if (!projected.state_.supporter_used ||
        projected.hand_count(current_supporter) >= current_before) {
      return false;
    }
    projected.issue_3545_continue_after_supporter(projected);
    if (projected.state_.turn_ended ||
        !remove_one(projected.state_.hand, Card::VsSeeker)) {
      return false;
    }

    projected.state_.discard.push_back(Card::VsSeeker);
    if (!projected.recover_discard_to_hand(future_supporter)) return false;
    projected.resolve_powerglass_end_turn();
    issue_3545_advance_public_state_one_turn(projected);
    if (!projected.issue_3545_recovered_supporter_is_used(
            projected, future_supporter)) {
      return false;
    }
    projected.issue_3545_continue_after_supporter(projected);
    return true;
  }

  std::vector<Card> issue_3545_bind_hidden_future_bank_targets(
      std::vector<Card> targets) const {
    if (targets.empty() || hand_count(Card::BattleCompressor) == 0 ||
        hand_count(Card::VsSeeker) < 2 || !supporter_allowed() ||
        state_.turn >= scenario_.max_turn ||
        !item_locked_on_turn(state_.turn + 1)) {
      return targets;
    }

    const auto current_supporter = issue_3545_compressor_supporter_target();
    if (!current_supporter ||
        issue_3545_hidden_stochastic_supporter(*current_supporter)) {
      return targets;
    }

    std::optional<Card> future_supporter;
    for (const Card target : targets) {
      if (!is_supporter(target) || target == *current_supporter) continue;
      future_supporter = target;
      break;
    }
    if (!future_supporter) return targets;

    if (issue_3545_future_bank_supporter_is_used(
            targets, *current_supporter, *future_supporter)) {
      return targets;
    }

    const auto it = std::find(targets.begin(), targets.end(), *future_supporter);
    if (it != targets.end()) targets.erase(it);
    return targets;
  }

  std::vector<Card> issue_3545_hidden_battle_compressor_targets() const {
    return issue_3545_bind_hidden_future_bank_targets(
        issue_3545_hidden_battle_compressor_targets_unbound());
  }

  bool play_battle_compressor_hidden_routes_issue3545_base() {
    const std::vector<Card> base_targets = issue_3545_battle_compressor_targets();
    const std::vector<Card> targets = issue_3545_hidden_battle_compressor_targets();
    if (targets.empty() || targets == base_targets) return false;

    if (!remove_one(state_.hand, Card::BattleCompressor)) {
      throw std::logic_error("bound hidden-route Battle Compressor disappeared");
    }
    state_.discard.push_back(Card::BattleCompressor);
    record_deck_search_knowledge("Battle Compressor");

    std::vector<Card> discarded;
    for (const Card target : targets) {
      if (!remove_one(state_.deck, target)) {
        throw std::logic_error("bound hidden-route Battle Compressor target disappeared");
      }
      state_.discard.push_back(target);
      state_.discarded_this_turn.push_back(target);
      discarded.push_back(target);
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-BATTLE-COMPRESSOR-01; R-GAME-ITEM; P-JIT-01; P-DCI-01; P-KNOWLEDGE-01",
          "Searched and discarded future-bank-bound targets: " +
              join_cards(discarded) + ".");
    return true;
  }

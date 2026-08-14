  void choose_supporter_issue3545_projection() {
    // Battle Compressor and VS Seeker can be evaluated from an early Item fixed point,
    // while the production turn still has deterministic setup actions before the one
    // Supporter play. Reproduce that same public sequence so the candidate is scored
    // after manual Energy, Forest Seal Stone, lock-removal, and Steven-specific
    // Powerglass attachment have reached the state the real Supporter chooser sees.
    // Strict and matchup-flex JIT also preserve payload through these early Item
    // passes, exactly as run_turn does.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Production turn order: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
    // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
    advance_forretress_combo();
    play_basics_from_hand();
    play_items_until_stable_issue_3057(!strict_payload_timing());
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable_issue_3057(!strict_payload_timing());
    attach_manual();
    if (should_play_steven()) attach_powerglass();

    const State before_supporter = state_;
    TraceLog projection_trace{true};
    TraceLog* const prior_trace = trace_;
    trace_ = &projection_trace;
    choose_supporter_issue_1152();
    trace_ = prior_trace;

    const auto trace_contains = [&projection_trace](const std::string_view text) {
      return std::any_of(
          projection_trace.lines.begin(), projection_trace.lines.end(),
          [text](const std::string& line) {
            return line.find(text) != std::string::npos;
          });
    };

    // Serena and Tate & Liza are deliberately modeled with fallback draw policy.
    // A BC/VS route chooser may decide to use one from the observable pre-draw state,
    // while it may not score the identities of the cards that the simulator happens
    // to draw from its hidden future RNG stream. Normalize those two draw effects to
    // their deterministic public consequences before the route score is read.
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Draw-Supporter fallback policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#draw-supporters
    const bool serena_draw = trace_contains("R-SERENA-01");
    const bool tate_draw = trace_contains("R-TATE-01") && trace_contains("draw mode");
    if (!serena_draw && !tate_draw) return;

    const State resolved = state_;
    state_ = before_supporter;

    if (tate_draw) {
      // Tate & Liza puts the remaining hand into the deck, then draws five. The
      // shuffled hand cards are known inputs to the deck composition; the five
      // future draw identities are intentionally absent from this projection.
      // This keeps a deterministic Tate switch route available because switch mode
      // never enters this branch.
      if (!remove_one(state_.hand, Card::TateLiza)) {
        throw std::logic_error("projected Tate & Liza draw target disappeared");
      }
      state_.discard.push_back(Card::TateLiza);
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      state_.hand.clear();
      state_.supporter_used = true;
      return;
    }

    // Serena's mandatory discard happens before its draw-to-five. Preserve every
    // deterministic discard selected by the ordinary DCI policy, including a legal
    // current-turn Dragon payload, while omitting only the unknown cards drawn after
    // those costs resolve. Serena does not shuffle the retained hand into the deck.
    std::vector<Card> new_discard = resolved.discard;
    for (const Card prior : before_supporter.discard) {
      if (!remove_one(new_discard, prior)) {
        throw std::logic_error("Serena projection discard provenance diverged");
      }
    }
    for (const Card discarded : new_discard) {
      if (!remove_one(state_.hand, discarded)) {
        throw std::logic_error("Serena projection hand discard disappeared");
      }
      state_.discard.push_back(discarded);
    }

    std::vector<Card> new_turn_discards = resolved.discarded_this_turn;
    for (const Card prior : before_supporter.discarded_this_turn) {
      if (!remove_one(new_turn_discards, prior)) {
        throw std::logic_error("Serena projection JIT provenance diverged");
      }
    }
    state_.discarded_this_turn.insert(
        state_.discarded_this_turn.end(),
        new_turn_discards.begin(), new_turn_discards.end());
    state_.supporter_used = true;
  }

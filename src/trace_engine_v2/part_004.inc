    state_.retreat_used = false;
    state_.turn_ended = false;
    state_.discarded_this_turn.clear();
    if (first_turn_going_first()) {
      trace("TURN START", "R-GAME-FIRST-TURN", "Going first: no opening-turn draw and no Supporter or attack.");
    } else {
      draw_card("start-of-turn");
    }
  }

  bool in_play(const Card card) const {
    if (state_.active && state_.active->card == card) return true;
    return std::any_of(state_.bench.begin(), state_.bench.end(), [card](const Pokemon& p) { return p.card == card; });
  }

  int in_play_count(const Card card) const {
    int count = state_.active && state_.active->card == card ? 1 : 0;
    count += static_cast<int>(std::count_if(state_.bench.begin(), state_.bench.end(), [card](const Pokemon& p) { return p.card == card; }));
    return count;
  }

  bool has_regi_v() const { return in_play(Card::RegidragoV); }
  bool has_vstar() const { return in_play(Card::RegidragoVstar); }
  bool has_any_regi() const { return has_regi_v() || has_vstar(); }
  bool active_is_vstar() const { return state_.active && state_.active->card == Card::RegidragoVstar; }
  int bench_space() const { return 5 - static_cast<int>(state_.bench.size()); }

  int score_regi(const Pokemon& p, const bool active) const {
    if (p.card != Card::RegidragoV && p.card != Card::RegidragoVstar) return -100000;
    return 100 * std::min(2, p.grass) + 100 * std::min(1, p.fire) +
           (p.card == Card::RegidragoVstar ? 15 : 0) + (active ? 3 : 0);
  }

  Pokemon* target_regi() {
    Pokemon* best = nullptr;
    int best_score = -100000;
    if (state_.active) {
      const int score = score_regi(*state_.active, true);
      if (score > best_score) { best = &*state_.active; best_score = score; }
    }
    for (Pokemon& p : state_.bench) {
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    return best;
  }

  const Pokemon* target_regi() const {
    const Pokemon* best = nullptr;
    int best_score = -100000;
    if (state_.active) {
      const int score = score_regi(*state_.active, true);
      if (score > best_score) { best = &*state_.active; best_score = score; }
    }
    for (const Pokemon& p : state_.bench) {
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    return best;
  }

  int grass_needed() const {
    const Pokemon* target = target_regi();
    return target == nullptr ? 2 : std::max(0, 2 - target->grass);
  }
  int fire_needed() const {
    const Pokemon* target = target_regi();
    return target == nullptr ? 1 : std::max(0, 1 - target->fire);
  }
  bool need_energy() const { return grass_needed() > 0 || fire_needed() > 0; }

  bool payload_ready() const {
    if (!strict_payload_timing()) return std::any_of(state_.discard.begin(), state_.discard.end(), is_payload);
    // A strict-JIT payload must both enter discard this turn and still be in discard
    // at the ready check. recover_discard_to_hand removes a recovered current-turn card.
    return std::any_of(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload);
  }
  bool need_payload() const { return !payload_ready(); }

  bool setup_axis_missing() const {
    return need_regi() || need_vstar() || need_energy() || need_active_vstar() || need_payload();
  }

  bool can_free_retreat_with_latias() const {
    return state_.active && is_basic(state_.active->card) && in_play(Card::LatiasEx) &&
           ability_available_for_pokemon(Card::LatiasEx) && !state_.retreat_used;
  }
  bool need_active_vstar() const { return has_vstar() && !active_is_vstar(); }

  std::string state_line() const {
    std::ostringstream out;
    out << "Active=";
    if (state_.active) out << pokemon_summary(*state_.active); else out << "(none)";
    out << " | Bench=";
    if (state_.bench.empty()) out << "(none)";
    for (std::size_t i = 0; i < state_.bench.size(); ++i) {
      if (i != 0) out << "; ";
      out << pokemon_summary(state_.bench[i]);
    }
    out << " | Hand=" << join_cards(state_.hand);

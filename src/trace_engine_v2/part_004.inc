    state_.retreat_used = false;
    state_.turn_ended = false;
    state_.discarded_this_turn.clear();
    // A player who has no cards left in their deck at the beginning of their turn
    // loses the game before taking turn actions: https://www.pokemon.com/us/pokemon-tcg/rules
    if (state_.deck.empty()) {
      state_.turn_ended = true;
      trace("GAME LOST", "R-GAME-DECK-OUT", "No cards remained in deck at the start of turn.");
      return;
    }
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

  // Crispin can advance the axis when an actually needed type remains in deck
  // at K1, or could still be unseen at K0. Card text source:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  bool crispin_can_advance_energy_axis() const {
    if (!need_energy()) return false;
    if (deck_seen_) {
      const bool grass_available = deck_count_after_search_started(Card::Grass) > 0;
      const bool fire_available = deck_count_after_search_started(Card::Fire) > 0;
      const bool needed_type_available = (grass_needed() > 0 && grass_available) ||
                                         (fire_needed() > 0 && fire_available);
      if (!needed_type_available) return false;
      // With one searchable Energy type, Crispin puts it into hand and attaches
      // none. It advances this model only while manual attachment remains open:
      // https://api.pokemontcg.io/v2/cards/sv7-133
      if (grass_available != fire_available) return !state_.manual_energy_used;
      return true;
    }
    return (grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
           (fire_needed() > 0 && might_be_unseen(Card::Fire));
  }

  // Gladion may take any one Prize card. This is deliberately narrower than
  // "any needed Energy": it is a direct route only when one manual attachment
  // completes every remaining modeled ready axis. Do not let it preempt a
  // missing VSTAR, active-position, or payload route.
  // Card text source: https://api.pokemontcg.io/v2/cards/sm4-95
  std::optional<Card> final_prized_energy_for_gladion() const {
    if (!prizes_known() || state_.manual_energy_used || grass_needed() + fire_needed() != 1 ||
        need_regi() || need_vstar() || need_active_vstar() || need_payload()) {
      return std::nullopt;
    }
    if (grass_needed() == 1 && prize_count_after_reveal(Card::Grass) > 0) return Card::Grass;
    if (fire_needed() == 1 && prize_count_after_reveal(Card::Fire) > 0) return Card::Fire;
    return std::nullopt;
  }

  bool payload_ready() const {
    // Readiness is defined exclusively by is_payload. Dipplin TWM 127 is a
    // legal Dragon search target, while maintainer policy excludes Syrup Catcher
    // from the A/S ready-state set: https://api.pokemontcg.io/v2/cards/sv6-127
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
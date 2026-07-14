    state_.retreat_used = false;
    state_.turn_ended = false;
    state_.discarded_this_turn.clear();
    // Drawing a card is the first step at the beginning of every turn, including
    // the first player's opening turn:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
    // The first player is restricted from playing a Supporter or attacking on that
    // turn; the restriction does not remove the required draw:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#First_Turn
    if (state_.deck.empty()) {
      state_.turn_ended = true;
      trace("GAME LOST", "R-GAME-DECK-OUT", "No cards remained in deck for the required start-of-turn draw.");
      return;
    }
    draw_card("start-of-turn");
    if (first_turn_going_first()) {
      trace("TURN START", "R-GAME-FIRST-TURN",
            "Going first: drew for the turn; no Supporter or attack is permitted.");
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
    const bool active_regi_can_evolve_from_hand = state_.turn > 1 && state_.active &&
        state_.active->card == Card::RegidragoV && state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0;
    // Prefer an existing Benched VSTAR only when it has a legal same-turn path to
    // the Active Spot. Latias ex gives a Basic Active no Retreat Cost, and Tate &
    // Liza can switch a Benched Pokemon Active:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool latias_promotion_route = !active && p.card == Card::RegidragoVstar &&
        !active_regi_can_evolve_from_hand && can_free_retreat_with_latias();
    // Tate & Liza consumes the Supporter play. Require the payload axis to be
    // complete so this preference does not displace Professor Burnet:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool tate_promotion_route = !active && p.card == Card::RegidragoVstar &&
        !active_regi_can_evolve_from_hand && supporter_allowed() &&
        hand_count(Card::TateLiza) > 0 && payload_ready();
    const int route_bonus = latias_promotion_route || tate_promotion_route ? 1000 : 0;
    return route_bonus + 100 * std::min(2, p.grass) + 100 * std::min(1, p.fire) +
           (p.card == Card::RegidragoVstar ? 15 : 0) + (active ? 3 : 0);
  }

  int benched_vstar_promotion_score(const Pokemon& pokemon) const {
    if (pokemon.card != Card::RegidragoVstar) return -1;
    // Apex Dragon requires two Grass and one Fire Energy. Promotion routes must
    // prefer a complete attacker, then the VSTAR with the most progress toward GGF;
    // stable Bench order remains the tie-break because replacement requires >:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return 100 * std::min(2, pokemon.grass) + 100 * std::min(1, pokemon.fire);
  }

  Pokemon* best_benched_vstar_for_promotion() {
    Pokemon* best = nullptr;
    int best_score = -1;
    for (Pokemon& pokemon : state_.bench) {
      const int score = benched_vstar_promotion_score(pokemon);
      if (score > best_score) {
        best = &pokemon;
        best_score = score;
      }
    }
    return best;
  }

  const Pokemon* best_benched_vstar_for_promotion() const {
    const Pokemon* best = nullptr;
    int best_score = -1;
    for (const Pokemon& pokemon : state_.bench) {
      const int score = benched_vstar_promotion_score(pokemon);
      if (score > best_score) {
        best = &pokemon;
        best_score = score;
      }
    }
    return best;
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
  // at K1, or could still be unseen at K0. A resolved Heavy Ball or Gladion
  // Prize inspection establishes K1 in this fixed-list model; with one Basic
  // Energy type left, Crispin puts it into hand and attaches none, so the
  // manual attachment must still be available: https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  bool crispin_can_advance_energy_axis() const {
    if (!need_energy()) return false;
    const bool grass_available = prizes_known() ? deck_count_after_search_started(Card::Grass) > 0
                                                : might_be_unseen(Card::Grass);
    const bool fire_available = prizes_known() ? deck_count_after_search_started(Card::Fire) > 0
                                               : might_be_unseen(Card::Fire);
    const bool needed_type_available = (grass_needed() > 0 && grass_available) ||
                                      (fire_needed() > 0 && fire_available);
    if (!needed_type_available) return false;
    // The K0 branch uses only public fixed-list counts through might_be_unseen().
    // If only one Basic Energy type can be searched, Crispin puts it into hand and
    // attaches none, so the route advances only while the manual attachment remains:
    // https://api.pokemontcg.io/v2/cards/sv7-133 https://www.pokemon.com/us/pokemon-tcg/rules
    if (grass_available != fire_available) return !state_.manual_energy_used;
    return true;
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
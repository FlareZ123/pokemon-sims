    int best_score = -100000;
    if (state_.active && state_.active->card == Card::RegidragoV && state_.active->entered_turn < state_.turn) {
      best = &*state_.active;
      best_score = score_regi(*best, true);
    }
    for (Pokemon& p : state_.bench) {
      if (p.card != Card::RegidragoV || p.entered_turn >= state_.turn) continue;
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    if (best == nullptr) return false;
    remove_one(state_.hand, Card::RegidragoVstar);
    best->card = Card::RegidragoVstar;
    trace("EVOLVE", "R-GAME-EVOLVE", "Evolved " + std::string(name(Card::RegidragoV)) + " into Regidrago VSTAR.");
    return true;
  }

  bool retreat_to_benched_vstar_with_latias() {
    if (!need_active_vstar() || !can_free_retreat_with_latias()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    std::swap(*state_.active, *it);
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT", "Latias ex gives the Basic Active no Retreat Cost; promoted Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_switch() {
    // Card text source: https://api.pokemontcg.io/v2/cards/sm7-148
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || !need_active_vstar()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    std::swap(*state_.active, *it);
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza switch mode: promoted Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_draw() {
    // Tate & Liza has no hand-size condition: shuffle the whole hand into the deck, then draw 5 cards: https://api.pokemontcg.io/v2/cards/sm7-148
    // This preserves legal five-or-more-card dead-hand refreshes after direct connectors fail.
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int draw = 0; draw < 5; ++draw) draw_card("Tate & Liza");
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza draw mode: shuffled remaining hand and drew 5.");
    return true;
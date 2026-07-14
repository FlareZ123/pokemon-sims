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
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    // Latias ex removes the Basic Active's Retreat Cost. Promote the Benched
    // Regidrago VSTAR that best satisfies Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT", "Latias ex gives the Basic Active no Retreat Cost; promoted the best-powered Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_switch() {
    // Card text source: https://api.pokemontcg.io/v2/cards/sm7-148
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || !need_active_vstar()) return false;
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    // Tate & Liza may switch any Benched Pokémon Active. Select the Regidrago
    // VSTAR that best satisfies Apex Dragon's GGF cost before spending Supporter:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    std::swap(*state_.active, *target);
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza switch mode: promoted the best-powered Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_draw() {
    // Tate & Liza has no hand-size condition: shuffle the whole hand into the deck, then draw 5 cards: https://api.pokemontcg.io/v2/cards/sm7-148
    // This preserves legal five-or-more-card dead-hand refreshes after direct connectors fail.
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0) return false;
    // After the played Supporter leaves hand, an empty deck plus no remaining hand
    // cards makes this draw mode guaranteed to do nothing, so the Trainer is illegal:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    if (state_.deck.empty() && state_.hand.size() == 1U) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int draw = 0; draw < 5; ++draw) draw_card("Tate & Liza");
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza draw mode: shuffled remaining hand and drew 5.");
    return true;
  }
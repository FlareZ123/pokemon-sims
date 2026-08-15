  bool promote_best_benched_vstar() {
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    std::swap(*state_.active, *target);
    return true;
  }

  bool retreat_to_benched_vstar_with_latias() {
    if (!need_active_vstar() || !can_free_retreat_with_latias()) return false;
    // Latias ex removes the Basic Active's Retreat Cost. Promote the Benched
    // Regidrago VSTAR that best satisfies Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (!promote_best_benched_vstar()) return false;
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT",
          "Latias ex gives the Basic Active no Retreat Cost; promoted the "
          "best-powered Benched Regidrago VSTAR.");
    return true;
  }

  bool tate_liza_available() const {
    // Tate & Liza is a Supporter, so availability is the shared Supporter-turn
    // legality gate plus the physical card in hand:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    return supporter_allowed() && hand_count(Card::TateLiza) > 0;
  }

  void consume_tate_liza_supporter() {
    // Tate & Liza is a Supporter; playing it consumes the turn's Supporter action:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
  }

  bool play_tate_switch() {
    // Card text source: https://api.pokemontcg.io/v2/cards/sm7-148
    if (!tate_liza_available() || !need_active_vstar()) return false;
    if (!promote_best_benched_vstar()) return false;
    consume_tate_liza_supporter();
    // Tate & Liza may switch any Benched Pokémon Active. Select the Regidrago
    // VSTAR that best satisfies Apex Dragon's GGF cost before spending Supporter:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER",
          "Tate & Liza switch mode: promoted the best-powered Benched Regidrago "
          "VSTAR.");
    return true;
  }

  bool play_tate_draw() {
    // Tate & Liza has no hand-size condition: shuffle the whole hand into the deck,
    // then draw 5 cards: https://api.pokemontcg.io/v2/cards/sm7-148
    // This preserves legal five-or-more-card dead-hand refreshes after direct connectors fail.
    if (!tate_liza_available()) return false;
    // After the played Supporter leaves hand, an empty deck plus no remaining hand
    // cards makes this draw mode guaranteed to do nothing, so the Trainer is illegal:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    if (state_.deck.empty() && state_.hand.size() == 1U) return false;
    consume_tate_liza_supporter();
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int draw = 0; draw < 5; ++draw) draw_card("Tate & Liza");
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER",
          "Tate & Liza draw mode: shuffled remaining hand and drew 5.");
    return true;
  }

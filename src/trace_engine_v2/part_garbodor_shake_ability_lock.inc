  int garbodor_unlocked_turn_{0};

  bool garbodor_shake_ability_lock_scenario() const {
    return scenario_.label.rfind("strict-jit-garbodor-shake-ability-lock/", 0) == 0 ||
        scenario_.label.rfind("garbodor-shake-ability-lock/", 0) == 0;
  }

  bool garbodor_shake_lock_established() const {
    if (!garbodor_shake_ability_lock_scenario() || state_.turn <= 0) return false;

    // Boost Shake ends the opponent's first turn after evolving into Garbodor.
    // Regidrago going first therefore keeps its own T1 Ability window; Regidrago
    // going second begins its T1 after the opponent has already established the lock.
    // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
    // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
    // Core turn and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
    return scenario_.going_first ? state_.turn >= 2 : state_.turn >= 1;
  }

  bool garbodor_abilities_locked() const {
    return garbodor_shake_lock_established() &&
        garbodor_unlocked_turn_ != state_.turn;
  }

  bool ability_available_for_pokemon(const Card card) const {
    // Garbotoxin suppresses Pokemon Abilities broadly, including non-Rule-Box
    // Pokemon such as Oricorio. Forest Seal Stone remains outside this predicate:
    // Star Alchemy is the Tool-granted VSTAR Power and keeps the repository's
    // existing Forest Seal Stone ruling/model.
    // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Core Ability and Tool procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (garbodor_abilities_locked()) return false;
    return ability_available_for_pokemon_pre_garbodor(card);
  }

  bool garbodor_unlock_has_live_ability_route(const bool supporter_will_be_spent) {
    if (!garbodor_abilities_locked() || !setup_axis_missing()) return false;

    const int previous_unlock_turn = garbodor_unlocked_turn_;
    garbodor_unlocked_turn_ = state_.turn;

    // Wonder Tag needs the Supporter action that an Arven -> Field Blower line has
    // already consumed. A held Field Blower may still unlock Wonder Tag normally.
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
    const bool wonder_tag_route = !supporter_will_be_spent &&
        hand_count(Card::TapuLeleGX) > 0 && !in_play(Card::TapuLeleGX) &&
        bench_space() > 0 && needs_tapu_connector();
    const bool vital_dance_route = hand_count(Card::Oricorio) > 0 &&
        !in_play(Card::Oricorio) && bench_space() > 0 && needs_oricorio_connector();
    const bool in_play_skyliner_route = need_active_vstar() &&
        can_free_retreat_with_latias();

    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
        });
    const bool held_skyliner_route = hand_count(Card::LatiasEx) > 0 &&
        !in_play(Card::LatiasEx) && bench_space() > 0 && !state_.retreat_used &&
        state_.active && is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV && prior_turn_benched_regi &&
        (hand_count(Card::RegidragoVstar) > 0 || might_be_unseen(Card::RegidragoVstar));

    const bool in_play_legacy_star_route = !state_.vstar_power_used && has_vstar() &&
        setup_axis_missing();
    const bool prior_turn_active_regi = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn;
    const bool future_legacy_star_route = !state_.vstar_power_used &&
        prior_turn_active_regi && setup_axis_missing() &&
        (hand_count(Card::RegidragoVstar) > 0 || might_be_unseen(Card::RegidragoVstar));

    // Dark Asset has immediate value when Crobat V can enter from hand and the
    // post-play hand will be below six cards. Garbotoxin itself does not block
    // Bench play; only the Ability is disabled until the Tool is removed.
    // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
    const bool dark_asset_route = hand_count(Card::CrobatV) > 0 && bench_space() > 0 &&
        !state_.dark_asset_used && state_.hand.size() <= 6U;

    garbodor_unlocked_turn_ = previous_unlock_turn;
    return wonder_tag_route || vital_dance_route || in_play_skyliner_route ||
        held_skyliner_route || in_play_legacy_star_route ||
        future_legacy_star_route || dark_asset_route;
  }

  bool play_garbodor_field_blower() {
    if (item_locked() || hand_count(Card::FieldBlower) == 0 ||
        !garbodor_abilities_locked() ||
        !garbodor_unlock_has_live_ability_route(false)) {
      return false;
    }

    // Field Blower spends one legal Tool/Stadium target on Garbodor's attached Tool.
    // The unlock lasts only for this player turn. The maximum-pressure scenario
    // assumes the opponent has another Tool ready and re-establishes Garbotoxin
    // before Regidrago's next turn; storing only the current turn implements that
    // automatic relock without inventing a persistent removal state.
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
    // Core Item, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
    remove_one(state_.hand, Card::FieldBlower);
    state_.discard.push_back(Card::FieldBlower);
    garbodor_unlocked_turn_ = state_.turn;
    trace("PLAY ITEM", "R-GAME-ITEM; P-GARBODOR-SHAKE-01",
          "Field Blower discarded Garbodor's attached Tool; Garbotoxin is off for this turn. The maximum-pressure model re-Tools Garbodor before the next player turn.");
    return true;
  }

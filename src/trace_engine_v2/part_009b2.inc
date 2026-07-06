                           (target == Card::DialgaGX && want_payload);
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM", "Searched a Basic Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    const Card fallback = fallback_quick_ball_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Basic Pokémon remained after search.");
    return true;
  }

  bool play_ultra_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
    // Ultra Ball discards 2 other cards from hand before searching a Pokémon: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_regi = need_regi() && bench_space() > 0;
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool want_oricorio = need_energy() && hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
                               bench_space() > 0 && needs_oricorio_connector();
    if (!want_regi && !want_vstar && !want_latias && !want_oricorio && !can_jit_discard) return false;

    const auto first = choose_discard(can_jit_discard, true);
    if (!first) return false;
    remove_one(state_.hand, *first);
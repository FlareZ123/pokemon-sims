        discard_from_hand(*extra, "Serena optional discard", "R-SERENA-01");
      }
    }
    // Serena permits up to three discards in every DCI profile. In strict JIT,
    // choose_discard(false, true) offers only already-safe non-payload fodder.
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-164
    if (scenario_.dci == DciProfile::StrictJit) {
      for (int number = 0; number < 2; ++number) {
        const auto extra = choose_discard(false, true);
        if (!extra || is_payload(*extra)) break;
        discard_from_hand(*extra, "Serena optional discard", "R-SERENA-01");
      }
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER", "Used Serena draw mode after discarding at least one card.");
    return true;
  }

  bool use_legacy_star() {
    if (state_.vstar_power_used || !has_vstar() || !ability_available_for_pokemon(Card::RegidragoVstar)) return false;
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Use only when no direct payload outlet or direct connector remains and the discard-7 can create a JIT payload line.
    const bool item_payload_outlet_in_hand = !item_locked() &&
        (hand_count(Card::BrilliantBlender) > 0 || hand_count(Card::MysteriousTreasure) > 0 ||
         hand_count(Card::QuickBall) > 0 || hand_count(Card::EarthenVessel) > 0 ||
         hand_count(Card::UltraBall) > 0);
    // Item-lock effects prevent Item cards from being played from hand, so locked
    // hand Items cannot suppress Legacy Star as the only random payload line:
    // https://api.pokemontcg.io/v2/cards/me2pt5-16
    // Professor Burnet is a live non-Item hand outlet only while the Supporter play
    // is available; after one Supporter is used, it cannot be played this turn:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool supporter_payload_outlet_in_hand = supporter_allowed() && hand_count(Card::ProfessorBurnet) > 0;
    const bool only_random_payload_line = need_payload() && !item_payload_outlet_in_hand && !supporter_payload_outlet_in_hand;
    if (!only_random_payload_line) return false;
    state_.vstar_power_used = true;
    outcome_.used_legacy_star = true;
    std::vector<Card> discarded;
    for (int card = 0; card < 7 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      state_.discard.push_back(top);
      state_.discarded_this_turn.push_back(top);
      discarded.push_back(top);
    }
    std::vector<Card> recovered;
    // Legacy Star may recover any card after discarding the top 7 cards: https://api.pokemontcg.io/v2/cards/swsh12-136
    // It does not inspect the remaining deck or Prize cards. At K0, recover a
    // Blender route only when a payload may still be unseen; do not branch on
    // actual hidden deck identities. See https://www.pokemon.com/us/pokemon-tcg/rules
    const bool blender_payload_line = need_payload() && !item_locked() && payload_might_be_in_deck();
    if (blender_payload_line && count_of(state_.discard, Card::BrilliantBlender) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::BrilliantBlender)) throw std::logic_error("Legacy Star Blender target disappeared");
      recovered.push_back(Card::BrilliantBlender);
    }
    // Professor Burnet can search the deck for up to 2 cards and discard them, so a
    // recovered copy is the live payload bridge when the Supporter slot is unused:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool burnet_payload_line = need_payload() && supporter_allowed() && payload_might_be_in_deck();
    if (burnet_payload_line && count_of(state_.discard, Card::ProfessorBurnet) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::ProfessorBurnet)) throw std::logic_error("Legacy Star Burnet target disappeared");
      recovered.push_back(Card::ProfessorBurnet);
    }

    // Legacy Star may return Latias ex from discard, and Skyliner then gives the
    // Basic Active no Retreat Cost so the powered Benched VSTAR can be promoted:
    // https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv8-76
    const bool latias_retreat_line = need_active_vstar() && state_.active && is_basic(state_.active->card) &&
        bench_space() > 0 && !in_play(Card::LatiasEx) && ability_available_for_pokemon(Card::LatiasEx);
    if (latias_retreat_line && count_of(state_.discard, Card::LatiasEx) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::LatiasEx)) throw std::logic_error("Legacy Star Latias target disappeared");
      recovered.push_back(Card::LatiasEx);
    }

    // Legacy Star can recover a two-card Item bridge when Evolution Incense and a
    // one-discard Item were just discarded. Evolution Incense fetches an Evolution
    // Dragon payload, and the recovered Item can discard that fetched payload from hand:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    const bool evolution_incense_payload_line = need_payload() && !item_locked() && payload_might_be_in_deck() &&
        count_of(state_.discard, Card::EvolutionIncense) > 0 && recovered.empty();
    if (evolution_incense_payload_line) {
      for (const Card outlet : {Card::MysteriousTreasure, Card::QuickBall, Card::EarthenVessel}) {
        if (count_of(state_.discard, outlet) == 0) continue;
        if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
        if (!recover_discard_to_hand(outlet)) throw std::logic_error("Legacy Star hand-discard outlet disappeared");
        recovered.push_back(Card::EvolutionIncense);
        recovered.push_back(outlet);
        break;
      }
    }

    // Recover energy only if it enables a manual attachment this turn. Do not recover a strict-JIT payload.
    for (const Card desired : {Card::Grass, Card::Fire, Card::Crispin, Card::Arven}) {
      if (recovered.size() >= 2U) break;
      if (is_payload(desired) && strict_payload_timing()) continue;
      const auto it = std::find(state_.discard.begin(), state_.discard.end(), desired);
      if (it == state_.discard.end()) continue;
      if (desired == Card::Grass && grass_needed() == 0) continue;
      if (desired == Card::Fire && fire_needed() == 0) continue;
      if (!recover_discard_to_hand(desired)) throw std::logic_error("Legacy Star recovery target disappeared");
      recovered.push_back(desired);
    }
    trace("LEGACY STAR", "R-RVS-01; R-VSTAR-01", "Discarded top 7: " + join_cards(discarded) + "; recovered: " + join_cards(recovered) + ".");
    return true;
  }

  bool use_celestial_roar() {
    // The player going first cannot attack on that player's first turn. Celestial
    // Roar has no printed restriction on later turns: https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    if (first_turn_going_first() || state_.turn_ended || !state_.active || state_.active->card != Card::RegidragoV) return false;
    // Celestial Roar costs one Colorless Energy. Any attached Energy pays it:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    if (state_.active->grass + state_.active->fire < 1) return false;
    std::vector<Card> processed;
    std::vector<Card> attached;
    std::vector<Card> discarded;
    for (int card = 0; card < 3 && !state_.deck.empty(); ++card) {
      const Card top = state_.deck.back();
      state_.deck.pop_back();
      processed.push_back(top);
      if (top == Card::Grass) {
        ++state_.active->grass;
        attached.push_back(top);
      } else if (top == Card::Fire) {
        ++state_.active->fire;
        attached.push_back(top);
      } else {
        state_.discard.push_back(top);
        state_.discarded_this_turn.push_back(top);
        discarded.push_back(top);
      }
    }
    state_.turn_ended = true;
    trace("ATTACK", "R-RV-01; R-GAME-ATTACK", "Regidrago V used Celestial Roar; processed top 3: " + join_cards(processed) + "; attached Energy: " + join_cards(attached) + "; remaining discarded: " + join_cards(discarded) + ".");
    return true;
  }

  void play_basics_from_hand() {
    bool changed = true;
    while (changed && bench_space() > 0) {
      changed = false;
      if (need_regi() && hand_count(Card::RegidragoV) > 0) {
        changed = bench_from_hand(Card::RegidragoV, false);
      } else if (in_play_count(Card::RegidragoV) < 2 && hand_count(Card::RegidragoV) > 0 && !has_vstar()) {
        changed = bench_from_hand(Card::RegidragoV, false);
      } else if (need_active_vstar() && ability_available_for_pokemon(Card::LatiasEx) &&
                 !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) > 0) {
        changed = bench_from_hand(Card::LatiasEx, false);
      } else if (bench_tapu_if_useful()) {
        changed = true;
      } else if (bench_oricorio_if_useful()) {
        changed = true;
      }
    }
  }

  bool run_search_items_one_step(const bool permit_payload) {
    if (item_locked()) return false;
    play_basics_from_hand();
    if (need_regi() && bench_space() > 0) {
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
      if (play_heavy_ball()) return true;
    }
    if (need_vstar()) {
      if (play_evolution_incense()) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
    }
    if (need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx)) {
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
    }
    if (need_energy()) {
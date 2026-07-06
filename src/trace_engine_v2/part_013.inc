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
    const bool only_random_payload_line = need_payload() && hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0 &&
                                          hand_count(Card::MysteriousTreasure) == 0 && hand_count(Card::QuickBall) == 0 && hand_count(Card::EarthenVessel) == 0 && hand_count(Card::UltraBall) == 0;
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
    if (first_turn_going_first() || state_.turn_ended || !state_.active || state_.active->card != Card::RegidragoV) return false;
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Celestial Roar costs one Colorless Energy. Any attached Energy pays it.
    if (state_.active->grass + state_.active->fire < 1) return false;
    // Attack policy: turn-one going-second Celestial Roar is used only when Steven's Resolve did not end the turn.
    if (state_.turn != 1 || scenario_.going_first) return false;
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
    if (need_regi()) {
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
    if (need_active_vstar() && ability_available_for_pokemon(Card::LatiasEx) && !in_play(Card::LatiasEx)) {
      if (play_quick_ball(permit_payload)) return true;
      if (play_mysterious_treasure(permit_payload)) return true;
      if (play_ultra_ball(permit_payload)) return true;
    }
    if (need_energy()) {
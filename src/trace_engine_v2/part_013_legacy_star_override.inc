  bool use_legacy_star() {
    if (state_.vstar_power_used || !has_vstar() || !ability_available_for_pokemon(Card::RegidragoVstar)) return false;
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Use only when no direct payload outlet or direct connector remains and the
    // discard-7 can create a JIT payload line.
    const bool permit_payload_cost = can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const auto one_discard_item_payable = [this, permit_payload_cost](const Card item) {
      if (hand_count(item) == 0) return false;
      // These Items require a card other than the played Item to pay their cost:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      return choose_discard(permit_payload_cost, true, true, item).has_value();
    };
    const auto ultra_ball_payable = [this, permit_payload_cost]() {
      if (hand_count(Card::UltraBall) == 0) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      const auto first = choose_discard(permit_payload_cost, true, true, Card::UltraBall);
      if (!first || !remove_one(state_.hand, *first)) {
        restore_hand();
        return false;
      }
      const auto second = choose_discard(can_play_payload_this_turn() && !is_payload(*first),
                                         true, true, Card::UltraBall);
      restore_hand();
      // Ultra Ball requires two other cards, so both selections must exist:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      return second.has_value();
    };
    const bool item_payload_outlet_in_hand = !item_locked() &&
        (hand_count(Card::BrilliantBlender) > 0 ||
         one_discard_item_payable(Card::MysteriousTreasure) ||
         one_discard_item_payable(Card::QuickBall) ||
         one_discard_item_payable(Card::EarthenVessel) ||
         ultra_ball_payable());
    // Item-lock effects prevent Item cards from being played from hand:
    // https://api.pokemontcg.io/v2/cards/me2pt5-16
    // A Supporter outlet is live only while the one Supporter play remains available:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool supporter_payload_outlet_in_hand = supporter_allowed() && hand_count(Card::ProfessorBurnet) > 0;
    // Legacy Star can create the strict-JIT payload only when the public deck count is
    // nonzero and a modeled payload may remain. At K1 the identity check is exact; at
    // K0 it uses public fixed-list counts without inspecting hidden card identities:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    const bool only_random_payload_line = need_payload() && !state_.deck.empty() &&
        payload_might_be_in_deck() && !item_payload_outlet_in_hand && !supporter_payload_outlet_in_hand;

    // Legacy Star may recover any two cards from discard, so an already-ready payload
    // does not make the VSTAR Power dead when a known recovery immediately completes a
    // different setup axis: https://api.pokemontcg.io/v2/cards/swsh12-136
    // A recovered Basic Energy can be manually attached once during the turn. The
    // selected VSTAR may already be Active or have a legal same-turn Latias retreat path:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const Pokemon* energy_target = target_regi();
    const bool selected_vstar_can_be_active_this_turn = active_is_vstar() ||
        (need_active_vstar() && can_free_retreat_with_latias() && energy_target != nullptr &&
         energy_target->card == Card::RegidragoVstar);
    const bool known_energy_completion = payload_ready() && selected_vstar_can_be_active_this_turn && need_energy() &&
        !state_.manual_energy_used &&
        ((grass_needed() > 0 && count_of(state_.discard, Card::Grass) > 0) ||
         (fire_needed() > 0 && count_of(state_.discard, Card::Fire) > 0));
    // A recovered Latias ex can be Benched, and Skyliner then gives the Basic Active no
    // Retreat Cost so a fully powered Benched VSTAR can be promoted this turn:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool known_latias_completion = payload_ready() && need_active_vstar() && !need_energy() &&
        state_.active && is_basic(state_.active->card) && bench_space() > 0 &&
        !in_play(Card::LatiasEx) && ability_available_for_pokemon(Card::LatiasEx) &&
        count_of(state_.discard, Card::LatiasEx) > 0;
    if (!only_random_payload_line && !known_energy_completion && !known_latias_completion) return false;

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
    // Legacy Star may recover any card after discarding the top 7 cards:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool blender_payload_line = need_payload() && !item_locked() && payload_might_be_in_deck();
    if (blender_payload_line && count_of(state_.discard, Card::BrilliantBlender) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::BrilliantBlender)) throw std::logic_error("Legacy Star Blender target disappeared");
      recovered.push_back(Card::BrilliantBlender);
    }
    const bool burnet_payload_line = need_payload() && supporter_allowed() && payload_might_be_in_deck();
    if (burnet_payload_line && count_of(state_.discard, Card::ProfessorBurnet) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::ProfessorBurnet)) throw std::logic_error("Legacy Star Burnet target disappeared");
      recovered.push_back(Card::ProfessorBurnet);
    }

    // Legacy Star may return Latias ex from discard, and Skyliner then gives the
    // Basic Active no Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-76
    const bool latias_retreat_line = need_active_vstar() && state_.active && is_basic(state_.active->card) &&
        bench_space() > 0 && !in_play(Card::LatiasEx) && ability_available_for_pokemon(Card::LatiasEx);
    if (latias_retreat_line && count_of(state_.discard, Card::LatiasEx) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::LatiasEx)) throw std::logic_error("Legacy Star Latias target disappeared");
      recovered.push_back(Card::LatiasEx);
    }

    // Evolution Incense plus a hand-discard outlet can form a two-card payload bridge:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    const bool evolution_incense_payload_line = need_payload() && !item_locked() && payload_might_be_in_deck() &&
        count_of(state_.discard, Card::EvolutionIncense) > 0 && recovered.empty();
    if (evolution_incense_payload_line) {
      // A later search Item needs one card to remain after Evolution Incense removes
      // the payload. Serena remains live with a one-card deck because its mandatory
      // discard is an effect and does not require another deck search:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // https://api.pokemontcg.io/v2/cards/swsh12-164
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
      const bool later_search_can_begin = state_.deck.size() > 1U;

      // Ultra Ball can follow Evolution Incense only when the fetched Dragon plus a
      // distinct policy-admissible hand card can pay its two-other-card cost:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      if (later_search_can_begin && count_of(state_.discard, Card::UltraBall) > 0) {
        const std::vector<Card> base_hand = state_.hand;
        const std::vector<Card> base_discard = state_.discard;
        const std::vector<Card> base_discarded_this_turn = state_.discarded_this_turn;
        const auto restore = [this, &base_hand, &base_discard, &base_discarded_this_turn]() {
          state_.hand = base_hand;
          state_.discard = base_discard;
          state_.discarded_this_turn = base_discarded_this_turn;
        };

        bool ultra_bridge_payable = false;
        if (recover_discard_to_hand(Card::EvolutionIncense) &&
            recover_discard_to_hand(Card::UltraBall)) {
          state_.hand.push_back(Card::MegaDragonite);
          if (!remove_one(state_.hand, Card::MegaDragonite)) {
            throw std::logic_error("Legacy Star representative payload disappeared");
          }
          ultra_bridge_payable =
              choose_discard(false, true, true, Card::UltraBall).has_value();
        }
        restore();

        if (ultra_bridge_payable) {
          if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
          if (!recover_discard_to_hand(Card::UltraBall)) throw std::logic_error("Legacy Star Ultra Ball target disappeared");
          recovered.push_back(Card::EvolutionIncense);
          recovered.push_back(Card::UltraBall);
        }
      }

      if (later_search_can_begin && recovered.empty()) {
        for (const Card outlet : {Card::MysteriousTreasure, Card::QuickBall, Card::EarthenVessel}) {
          if (count_of(state_.discard, outlet) == 0) continue;
          if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
          if (!recover_discard_to_hand(outlet)) throw std::logic_error("Legacy Star hand-discard outlet disappeared");
          recovered.push_back(Card::EvolutionIncense);
          recovered.push_back(outlet);
          break;
        }
      }

      // Serena's first mode requires discarding at least one hand card before drawing
      // to five. After Incense fetches the Dragon, that payload is a guaranteed legal
      // discard while the one Supporter play remains open:
      // https://api.pokemontcg.io/v2/cards/swsh12-164
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      if (recovered.empty() && supporter_allowed() && count_of(state_.discard, Card::Serena) > 0) {
        if (!recover_discard_to_hand(Card::EvolutionIncense)) throw std::logic_error("Legacy Star Evolution Incense target disappeared");
        if (!recover_discard_to_hand(Card::Serena)) throw std::logic_error("Legacy Star Serena target disappeared");
        recovered.push_back(Card::EvolutionIncense);
        recovered.push_back(Card::Serena);
      }
    }

    // Earthen Vessel can turn one or two Legacy Star recoveries into the exact Basic
    // Energy needed for the unused manual attachment. Recover Vessel alone when the
    // hand already supplies its discard cost, or recover a second policy-admissible
    // cost card from the newly discarded cards: https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // A permitted Dragon payload is the strongest strict-JIT Vessel cost because the
    // required discard also establishes Apex Dragon's current-turn payload axis:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool needed_energy_might_remain =
        (grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
        (fire_needed() > 0 && might_be_unseen(Card::Fire));
    if (recovered.size() < 2U && need_energy() && !item_locked() && !state_.manual_energy_used &&
        needed_energy_might_remain && count_of(state_.discard, Card::EarthenVessel) > 0) {
      const std::vector<Card> base_hand = state_.hand;
      const std::vector<Card> base_discard = state_.discard;
      const std::vector<Card> base_discarded_this_turn = state_.discarded_this_turn;
      const auto restore = [this, &base_hand, &base_discard, &base_discarded_this_turn]() {
        state_.hand = base_hand;
        state_.discard = base_discard;
        state_.discarded_this_turn = base_discarded_this_turn;
      };

      bool found_vessel_bridge = false;
      if (recover_discard_to_hand(Card::EarthenVessel) &&
          choose_discard(can_play_payload_this_turn(), true, true, Card::EarthenVessel).has_value()) {
        recovered.push_back(Card::EarthenVessel);
        found_vessel_bridge = true;
      } else {
        restore();
      }

      if (!found_vessel_bridge && recovered.empty()) {
        for (const Card candidate : base_discard) {
          restore();
          if (!recover_discard_to_hand(Card::EarthenVessel) || !recover_discard_to_hand(candidate)) continue;
          if (!choose_discard(can_play_payload_this_turn(), true, true, Card::EarthenVessel).has_value()) continue;
          recovered.push_back(Card::EarthenVessel);
          recovered.push_back(candidate);
          found_vessel_bridge = true;
          break;
        }
      }
      if (!found_vessel_bridge) restore();
    }

    // Recover Basic Energy for a live manual attachment. Crispin and Arven cannot
    // resolve after a spent Supporter slot, though either remains a valid future-turn
    // fallback when no payable same-turn recovery bridge above exists:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://www.pokemon.com/us/pokemon-tcg/rules
    for (const Card desired : {Card::Grass, Card::Fire, Card::Crispin, Card::Arven}) {
      if (recovered.size() >= 2U) break;
      if (is_payload(desired) && strict_payload_timing()) continue;
      if ((desired == Card::Grass || desired == Card::Fire) && state_.manual_energy_used) continue;
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

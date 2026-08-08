  bool held_payload_only_outlet_blocks_combined_legacy_star_route() const {
    // These held cards can establish a current-turn Dragon payload, but none can
    // supply the missing Basic Energy by itself. They therefore cannot suppress a
    // Legacy Star line that is needed to bridge both axes:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Confirmed non-Blender regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
    const bool held_item_payload_only_outlet = !item_locked() &&
        (hand_count(Card::BrilliantBlender) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0);
    const bool held_supporter_payload_only_outlet =
        supporter_allowed() && hand_count(Card::ProfessorBurnet) > 0;
    if ((!held_item_payload_only_outlet && !held_supporter_payload_only_outlet) ||
        !deck_seen_ || !active_is_vstar() || state_.manual_energy_used ||
        !need_energy() || !need_payload() || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck() || state_.deck.empty()) {
      return false;
    }

    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    if (missing_grass + missing_fire != 1) return false;

    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    if (hand_count(needed_energy) > 0) return false;

    // Admission must be deterministic before Legacy Star reveals its seven cards.
    // A needed Energy already in discard is directly recoverable. Otherwise, an
    // Earthen Vessel already in discard is the public fallback when the needed
    // Energy remains in deck: if Legacy Star mills that Energy it can recover it,
    // and if it does not, it can recover Vessel and search the still-present Energy.
    // This avoids spending the one-use VSTAR Power merely because hidden top-seven
    // order happens to reveal a favorable Vessel or Energy, while preserving the
    // exact K1 seed-4 proof where Vessel is public before the Ability is used:
    // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Hidden-information / K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing deterministic Vessel-Burnet route preserved by this boundary: https://github.com/FlareZ123/pokemon-sims/issues/1866
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
    const bool direct_energy_recovery =
        count_of(state_.discard, needed_energy) > 0;
    const bool public_vessel_fallback =
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        count_of(state_.deck, needed_energy) > 0;
    return direct_energy_recovery || public_vessel_fallback;
  }

  bool issue_2314_latias_energy_projection_available() {
    if (!deck_seen_ || state_.vstar_power_used || !has_vstar() ||
        !ability_available_for_pokemon(Card::RegidragoVstar) || item_locked() ||
        state_.manual_energy_used || state_.retreat_used || !need_energy() ||
        !need_active_vstar() || !state_.active || !is_basic(state_.active->card) ||
        bench_space() <= 0 || in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 || hand_count(Card::MysteriousTreasure) == 0 ||
        !can_play_payload_this_turn()) {
      return false;
    }

    Pokemon* target = target_regi();
    if (target == nullptr || target->card != Card::RegidragoVstar) return false;
    const int missing_grass = std::max(0, 2 - target->grass);
    const int missing_fire = std::max(0, 1 - target->fire);
    if (missing_grass + missing_fire != 1) return false;
    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    if (hand_count(needed_energy) > 0) return false;

    const bool latias_in_deck = count_of(state_.deck, Card::LatiasEx) > 0;
    const bool latias_in_discard = count_of(state_.discard, Card::LatiasEx) > 0;
    if (!latias_in_deck && !latias_in_discard) return false;
    if (count_of(state_.discard, Card::EarthenVessel) == 0 ||
        count_of(state_.deck, needed_energy) + count_of(state_.discard, needed_energy) == 0) {
      return false;
    }

    // If Latias is milled, held Mysterious Treasure becomes the payload outlet and
    // still needs a legal Psychic/Dragon target. Seven other targets are sufficient
    // when Latias begins in deck because a seven-card Legacy Star discard cannot
    // remove Latias plus all seven; if Latias was already discarded, eight are needed.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // K1 hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed deterministic projection: https://github.com/FlareZ123/pokemon-sims/issues/2314
    const int other_treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), [](const Card card) {
          return card != Card::LatiasEx && is_dragon_or_psychic(card);
        }));
    const int required_other_targets = latias_in_deck ? 7 : 8;
    if (other_treasure_targets < required_other_targets) return false;

    // The worst reveal branch leaves both Latias and the needed Energy in deck, so
    // recovered Earthen Vessel and held Mysterious Treasure need two distinct costs.
    // The first cost establishes the strict-JIT Dragon payload. For the second cost,
    // route-aware DCI may spend a held Basic Energy whose type is already fully paid
    // on the selected VSTAR because the observable current-turn route replaces that
    // card's setup role. This is the seed-15 Fire: the VSTAR already has F and needs G.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Apex Dragon / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dynamic DCI and route-replaced resources: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed seed-15 state: https://github.com/FlareZ123/pokemon-sims/issues/2314
    const auto payload_cost = choose_discard(true, true, true, Card::EarthenVessel);
    if (!payload_cost || !is_payload(*payload_cost)) return false;
    const bool route_replaced_energy_cost =
        (missing_grass == 0 && hand_count(Card::Grass) > 0) ||
        (missing_fire == 0 && hand_count(Card::Fire) > 0);
    if (!route_replaced_energy_cost) {
      const std::vector<Card> original_hand = state_.hand;
      const std::vector<Card> original_discard = state_.discard;
      const std::vector<Card> original_discarded_this_turn = state_.discarded_this_turn;
      if (!remove_one(state_.hand, *payload_cost)) return false;
      state_.discard.push_back(*payload_cost);
      state_.discarded_this_turn.push_back(*payload_cost);
      const auto ordinary_second_cost = choose_discard(
          false, true, true, Card::MysteriousTreasure);
      state_.hand = original_hand;
      state_.discard = original_discard;
      state_.discarded_this_turn = original_discarded_this_turn;
      if (!ordinary_second_cost) return false;
    }
    return true;
  }

  bool use_issue_2314_latias_energy_projection() {
    if (!issue_2314_latias_energy_projection_available()) return false;

    Pokemon* target = target_regi();
    if (target == nullptr || target->card != Card::RegidragoVstar) {
      throw std::logic_error("Issue 2314 projected VSTAR target disappeared");
    }
    const Card needed_energy = target->grass < 2 ? Card::Grass : Card::Fire;

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
    if (count_of(state_.discard, Card::LatiasEx) > 0) {
      if (!recover_discard_to_hand(Card::LatiasEx)) {
        throw std::logic_error("Issue 2314 Legacy Star Latias target disappeared");
      }
      recovered.push_back(Card::LatiasEx);
    }
    if (count_of(state_.discard, needed_energy) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(needed_energy)) {
        throw std::logic_error("Issue 2314 Legacy Star Energy target disappeared");
      }
      recovered.push_back(needed_energy);
    } else if (count_of(state_.deck, needed_energy) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Issue 2314 Legacy Star Vessel target disappeared");
      }
      recovered.push_back(Card::EarthenVessel);
    }

    // Legacy Star's two-card recovery budget covers every legal reveal branch:
    // recover Latias if it was discarded, recover the missing Basic Energy if it was
    // discarded, otherwise recover the already-discarded Vessel to search that Energy.
    // Held Treasure searches Latias whenever it remains in deck, and Skyliner then
    // gives the Basic Active free Retreat so the powered Benched VSTAR can be promoted.
    // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Official Ability, Item, discard, search, attachment, Bench, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2314
    trace("LEGACY STAR", "R-RVS-01; R-VSTAR-01",
          "Discarded top 7: " + join_cards(discarded) + "; recovered: " +
              join_cards(recovered) + ".");
    return true;
  }

  bool refine_issue_2315_legacy_star_tate_recovery(
      const bool used, const int grass_before, const int fire_before,
      const int vessel_before) {
    if (!used || !deck_seen_ || item_locked() || state_.manual_energy_used ||
        !supporter_allowed() || !need_active_vstar() || !state_.active ||
        !is_basic(state_.active->card) || count_of(state_.discard, Card::TateLiza) == 0 ||
        hand_count(Card::EarthenVessel) <= vessel_before) {
      return false;
    }

    Pokemon* target = target_regi();
    if (target == nullptr || target->card != Card::RegidragoVstar) return false;
    const int missing_grass = std::max(0, 2 - target->grass);
    const int missing_fire = std::max(0, 1 - target->fire);
    if (missing_grass + missing_fire != 1) return false;
    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    const int needed_energy_before = needed_energy == Card::Grass ? grass_before : fire_before;
    if (hand_count(needed_energy) <= needed_energy_before ||
        count_of(state_.deck, needed_energy) == 0) {
      return false;
    }

    const std::vector<Card> original_hand = state_.hand;
    const std::vector<Card> original_discard = state_.discard;
    const std::vector<Card> original_discarded_this_turn = state_.discarded_this_turn;
    if (!remove_one(state_.hand, needed_energy)) return false;
    state_.discard.push_back(needed_energy);
    if (!recover_discard_to_hand(Card::TateLiza)) {
      state_.hand = original_hand;
      state_.discard = original_discard;
      state_.discarded_this_turn = original_discarded_this_turn;
      return false;
    }

    // Earthen Vessel still needs a legal non-Tate cost after the recovery swap. If
    // strict JIT is unresolved, that cost must itself establish the Dragon payload.
    // Tate & Liza remains protected for its independent switch axis:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Mega Dragonite ex, a modeled payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Apex Dragon / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dynamic DCI / strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed post-reveal recovery bug: https://github.com/FlareZ123/pokemon-sims/issues/2315
    const bool payload_needed_before_vessel = need_payload();
    const auto tate = std::find(state_.hand.begin(), state_.hand.end(), Card::TateLiza);
    if (tate == state_.hand.end()) throw std::logic_error("Issue 2315 Tate recovery disappeared");
    const Card protected_tate = *tate;
    state_.hand.erase(tate);
    const auto vessel_cost = choose_discard(
        can_play_payload_this_turn(), true, true, Card::EarthenVessel);
    state_.hand.push_back(protected_tate);
    const bool vessel_route_complete = vessel_cost.has_value() &&
        (!payload_needed_before_vessel || is_payload(*vessel_cost));
    if (!vessel_route_complete) {
      state_.hand = original_hand;
      state_.discard = original_discard;
      state_.discarded_this_turn = original_discarded_this_turn;
      return false;
    }

    // Legacy Star selects its recovery cards after the seven discarded cards are
    // public. Vessel already searches the final Basic Energy, so Tate & Liza has
    // strictly greater discrete value than the redundant direct Energy recovery: it
    // completes the still-missing Active-position axis with the unused Supporter play.
    // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Tate & Liza switch mode: https://api.pokemontcg.io/v2/cards/sm7-148
    // Dialga-GX Basic Active: https://api.pokemontcg.io/v2/cards/sm5-100
    // Official Supporter, Item, discard, search, attachment, and switch procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 / earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Existing Vessel bridge: https://github.com/FlareZ123/pokemon-sims/issues/417
    // Related hidden-order Tate projection: https://github.com/FlareZ123/pokemon-sims/issues/1130
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2315
    trace("LEGACY STAR RECOVERY", "R-RVS-01; R-GAME-SUPPORTER",
          "Replaced redundant " + std::string(name(needed_energy)) +
              " recovery with Tate & Liza because recovered Earthen Vessel can search the Energy and Tate completes the Active-position axis.");
    return true;
  }

  bool use_legacy_star() {
    if (use_issue_2314_latias_energy_projection()) return true;

    const bool issue_1817_hold_for_burnet_finish =
        issue_1817_burnet_finish_pending_ &&
        scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::TurnTwoItem && state_.turn == 2 &&
        scenario_.max_turn >= 3 && deck_seen_ && state_.manual_energy_used &&
        active_is_vstar() && state_.active->grass == 1 &&
        state_.active->fire == 1 && hand_count(Card::Grass) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 && need_payload();
    if (issue_1817_hold_for_burnet_finish) {
      // VSTAR, the held final Grass, and held Professor Burnet already guarantee
      // T3 readiness. Preserve the one-use VSTAR Power instead of introducing a
      // random discard and recovery sequence that cannot improve the deadline:
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official VSTAR Power and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Resource-preservation and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1817
      return false;
    }

    const bool issue_1765_hold_for_t3_payload_window =
        scenario_.dci == DciProfile::StrictJit && scenario_.locks == LockMode::None &&
        !scenario_.going_first && state_.turn == 2 && scenario_.max_turn >= 3 &&
        deck_seen_ && state_.manual_energy_used && active_is_vstar() &&
        grass_needed() == 0 && fire_needed() == 1 && need_payload() &&
        hand_count(Card::Fire) > 0 && hand_count(Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0;
    if (issue_1765_hold_for_t3_payload_window) {
      // The surplus-Grass Treasure route completes the VSTAR and Grass axes on T2,
      // while the held Fire completes GGF on T3. Strict JIT cannot credit a T2
      // Legacy Star payload on T3, so preserve the one-use VSTAR Power for the first
      // turn that can complete every setup axis:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // VSTAR Power and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1765
      return false;
    }

    const int grass_before = hand_count(Card::Grass);
    const int fire_before = hand_count(Card::Fire);
    const int vessel_before = hand_count(Card::EarthenVessel);
    if (!held_payload_only_outlet_blocks_combined_legacy_star_route()) {
      const bool used = use_legacy_star_issue1069_original();
      refine_issue_2315_legacy_star_tate_recovery(
          used, grass_before, fire_before, vessel_before);
      return used;
    }

    const int held_blenders = hand_count(Card::BrilliantBlender);
    const int held_treasures = hand_count(Card::MysteriousTreasure);
    const int held_burnets = hand_count(Card::ProfessorBurnet);
    std::erase(state_.hand, Card::BrilliantBlender);
    std::erase(state_.hand, Card::MysteriousTreasure);
    std::erase(state_.hand, Card::ProfessorBurnet);
    const bool used = use_legacy_star_issue1069_original();
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_blenders),
                       Card::BrilliantBlender);
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_treasures),
                       Card::MysteriousTreasure);
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_burnets),
                       Card::ProfessorBurnet);
    refine_issue_2315_legacy_star_tate_recovery(
        used, grass_before, fire_before, vessel_before);

    // Legacy Star may recover any two cards after its seven-card discard. When a
    // payload-only outlet is the sole preflight suppressor and a public K1 Energy
    // recovery bridge already exists, temporarily excluding that outlet lets the
    // existing adaptive recovery policy evaluate the missing Energy axis. The held
    // cards are restored immediately after the VSTAR Power evaluation, so later
    // same-turn play can still use them when required:
    // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Official Item, Supporter, Ability, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Dynamic DCI and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Original Blender regression: https://github.com/FlareZ123/pokemon-sims/issues/1069
    // Confirmed non-Blender regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
    return used;
  }
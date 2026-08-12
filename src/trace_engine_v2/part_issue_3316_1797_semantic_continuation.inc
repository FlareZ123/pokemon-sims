  bool issue_3316_1797_bank_continuation() {
    if (issue_1797_finish_turn_ != 0) return false;

    const bool tapu_in_play = in_play(Card::TapuLeleGX);
    const bool quick_ball_paid = count_of(state_.discard, Card::QuickBall) > 0 &&
        (count_of(state_.discard, Card::TateLiza) > 0 ||
         count_of(state_.discard, Card::Lusamine) > 0);
    const bool payload_available = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload) ||
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);

    // After the earlier Quick Ball -> Tapu Lele-GX connector has established K1,
    // this bank is governed by the current legal actions and its one-turn horizon.
    // StrictJit and MatchupFlexJit share the same ready-turn Dragon-discard rule.
    // Rule Box Ability lock leaves Steven's Resolve and the following-turn Earthen
    // Vessel legal; Item/combined locks remove the required Vessel continuation.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, attachment, evolution, search, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced Supporter, Item, attachment, evolution, search, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Turn-two Item lock remains active from the player's second turn onward: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // K1/JIT/lock/route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1797
    // Separate earlier selector scope: https://github.com/FlareZ123/pokemon-sims/issues/3165
    // Confirmed semantic-continuation bug: https://github.com/FlareZ123/pokemon-sims/issues/3316
    const bool route_available = strict_payload_timing() &&
        state_.turn + 1 <= scenario_.max_turn && prizes_known() &&
        supporter_allowed() && !state_.manual_energy_used && !item_locked() &&
        !item_locked_on_turn(state_.turn + 1) && // Persistent projected Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
        state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 0 && tapu_in_play &&
        quick_ball_paid && hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Grass) > 0 && hand_count(Card::StevensResolve) > 0 &&
        (hand_count(Card::Crispin) > 0 ||
         deck_count_after_search_started(Card::Crispin) > 0) &&
        (hand_count(Card::EarthenVessel) > 0 ||
         deck_count_after_search_started(Card::EarthenVessel) > 0) &&
        payload_available;
    if (!route_available) return false;

    remove_one(state_.hand, Card::Grass);
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH ENERGY", "R-GAME-ENERGY; P-COMPRESS-01",
          "Attached the held second Grass Energy before Steven's Resolve ends the bank turn.");

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-3316 semantic package");

    std::vector<Card> found;
    if (hand_count(Card::Crispin) == 0) {
      if (!move_deck_to_hand(Card::Crispin)) {
        throw std::logic_error("Issue-3316 Crispin target disappeared");
      }
      found.push_back(Card::Crispin);
    }
    if (hand_count(Card::EarthenVessel) == 0) {
      if (!move_deck_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Issue-3316 Earthen Vessel target disappeared");
      }
      found.push_back(Card::EarthenVessel);
    }
    if (!std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      const auto payload_it = std::find_if(
          state_.deck.begin(), state_.deck.end(), is_payload);
      if (payload_it == state_.deck.end()) {
        throw std::logic_error("Issue-3316 Dragon payload target disappeared");
      }
      const Card payload = *payload_it;
      if (!move_deck_to_hand(payload)) {
        throw std::logic_error("Issue-3316 Dragon payload search failed");
      }
      found.push_back(payload);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    issue_1797_finish_turn_ = state_.turn + 1;

    // Steven searches the missing Crispin, Vessel, and Dragon pieces and ends the
    // current turn. The stored finish turn is relative, so equivalent later turns
    // and either seat remain legal when every action-specific gate above is met.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3316
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Banked the issue-3316 next-turn Crispin, Vessel, and Dragon package: " +
              join_cards(found) + ". Turn ended.");
    return true;
  }

  bool issue_3316_1797_finish_continuation() {
    if (issue_1797_finish_turn_ != state_.turn) return false;

    std::optional<Card> payload;
    for (const Card card : state_.hand) {
      if (is_payload(card)) {
        payload = card;
        break;
      }
    }
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const int crispin_search_count =
        (deck_fire > 0 ? 1 : 0) + (deck_grass > 0 ? 1 : 0);
    const bool fire_accessible = hand_count(Card::Fire) > 0 || deck_fire > 0;
    const bool vessel_target_survives =
        deck_grass + deck_fire > crispin_search_count;

    // The bank marker already proves this is the immediately following player turn.
    // Current Supporter, Item, evolution, attachment, K1, and resource gates decide
    // legality. Rule Box Ability lock does not suppress Crispin or Earthen Vessel.
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, Supporter, Item, attachment, discard, and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced Supporter, Item, evolution, attachment, discard, and search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed semantic-continuation bug: https://github.com/FlareZ123/pokemon-sims/issues/3316
    const bool route_available = strict_payload_timing() &&
        state_.turn <= scenario_.max_turn && prizes_known() &&
        !state_.turn_ended && supporter_allowed() && !state_.manual_energy_used &&
        !item_locked() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Crispin) > 0 &&
        hand_count(Card::EarthenVessel) > 0 && payload.has_value() &&
        fire_accessible && crispin_search_count > 0 && vessel_target_survives;
    if (!route_available) {
      issue_1797_finish_turn_ = 0;
      return false;
    }

    evolve_best_regi();
    if (!active_is_vstar()) {
      throw std::logic_error("Issue-3316 Active Regidrago V did not evolve");
    }

    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;
    record_deck_search_knowledge("Crispin issue-3316 semantic finish");
    bool searched_fire = false;
    bool searched_grass = false;
    if (deck_fire > 0 && move_deck_to_hand(Card::Fire)) {
      searched_fire = true;
    }
    if (deck_grass > 0 && move_deck_to_hand(Card::Grass)) {
      searched_grass = true;
    }
    shuffle(state_.deck);

    if (searched_fire) {
      remove_one(state_.hand, Card::Fire);
      ++state_.active->fire;
      trace("CRISPIN ATTACH", "R-CRISPIN-01; R-GAME-ENERGY",
            "Searched Fire and attached it to the Active Regidrago VSTAR.");
    } else {
      if (hand_count(Card::Fire) == 0) {
        throw std::logic_error("Issue-3316 held Fire disappeared");
      }
      remove_one(state_.hand, Card::Fire);
      ++state_.active->fire;
      state_.manual_energy_used = true;
      trace("ATTACH ENERGY", "R-GAME-ENERGY; R-CRISPIN-01",
            "Used the held Fire because the intervening draw removed Fire from Crispin's deck targets.");
    }
    trace("PLAY SUPPORTER", "R-CRISPIN-01; R-GAME-SUPPORTER",
          std::string("Crispin searched ") +
              (searched_fire ? "Fire" : "no Fire") + " and " +
              (searched_grass ? "Grass" : "no Grass") + ".");

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*payload, "Earthen Vessel issue-3316 Dragon cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-3316 Vessel payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-3316 semantic finish");
    bool vessel_found = false;
    if (move_deck_to_hand(Card::Grass)) vessel_found = true;
    if (move_deck_to_hand(Card::Fire)) vessel_found = true;
    if (!vessel_found) {
      throw std::logic_error("Issue-3316 Vessel Energy target disappeared");
    }
    shuffle(state_.deck);
    issue_1797_finish_turn_ = 0;

    // Vessel pays the Steven-searched Dragon as its mandatory discard cost during
    // this ready turn, satisfying both same-ready-turn JIT profiles.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, Supporter, attachment, Item, discard, search, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3316
    trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01",
          "Discarded the held Dragon payload and searched the remaining Basic Energy.");
    return active_is_vstar() && state_.active->grass >= 2 &&
        state_.active->fire >= 1 && payload_ready();
  }

  const Pokemon* issue_1772_prepared_fire_regidrago() const {
    const auto found = std::find_if(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass == 0 &&
              pokemon.fire >= 1;
        });
    return found == state_.bench.end() ? nullptr : &*found;
  }

  bool issue_1772_steven_t3_package_available() const {
    const bool payload_in_known_deck =
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    const bool payload_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);

    // Full Prize inspection establishes the same fixed-list K1 as a resolved
    // deck search, so this route must use the repository-wide knowledge predicate:
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2066
    if (!prizes_known() || !strict_payload_timing() || !supporter_allowed() ||
        item_locked() || state_.turn != 2 || state_.turn + 1 > scenario_.max_turn ||
        !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV || state_.retreat_used ||
        bench_space() == 0 || issue_1772_prepared_fire_regidrago() == nullptr ||
        !need_energy() || !need_payload() || state_.manual_energy_used ||
        hand_count(Card::StevensResolve) == 0 || hand_count(Card::Gladion) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Grass) > 0 ||
        hand_count(Card::LatiasEx) > 0 || in_play(Card::LatiasEx) || payload_held ||
        !ability_available_for_pokemon(Card::LatiasEx)) {
      return false;
    }

    // Steven banks the three public cards that complete the next-turn route.
    // Crispin attaches one searched Grass, Steven's Grass is the manual attachment,
    // Earthen Vessel discards Steven's Dragon payload, the held VSTAR evolves the
    // prior-turn Basic, and Latias ex promotes it. Gladion taking the prized ACE SPEC
    // solves only the payload axis and loses this deterministic compression line:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official search, Supporter, Item, attachment, evolution, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, supporter contention, JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1772
    return deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 && payload_in_known_deck;
  }

  bool play_issue_1772_steven_t3_package() {
    if (!issue_1772_steven_t3_package_available()) return false;

    const auto payload = std::find_if(state_.deck.begin(), state_.deck.end(), is_payload);
    if (payload == state_.deck.end()) {
      throw std::logic_error("Issue-1772 payload disappeared");
    }
    const Card selected_payload = *payload;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1772 route");

    if (!move_deck_to_hand(Card::Grass) || !move_deck_to_hand(Card::LatiasEx) ||
        !move_deck_to_hand(selected_payload)) {
      throw std::logic_error("Issue-1772 Steven target disappeared");
    }
    shuffle(state_.deck);

    // Steven's printed "up to 3" search selects the exact Energy, promotion, and
    // discard-payload package and then ends T2:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://github.com/FlareZ123/pokemon-sims/issues/1772
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the deterministic T3 Grass-Latias-payload package. Turn ended.");
    return true;
  }

  bool issue_1772_t3_crispin_completion_available() const {
    const bool payload_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    return deck_seen_ && strict_payload_timing() && supporter_allowed() &&
        !item_locked() && state_.turn == 3 && state_.active &&
        is_basic(state_.active->card) && state_.active->card != Card::RegidragoV &&
        !state_.retreat_used && bench_space() > 0 &&
        issue_1772_prepared_fire_regidrago() != nullptr &&
        hand_count(Card::Gladion) > 0 && hand_count(Card::Crispin) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Grass) > 0 &&
        hand_count(Card::LatiasEx) > 0 && payload_held &&
        !state_.manual_energy_used && need_energy() && need_payload() &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
  }

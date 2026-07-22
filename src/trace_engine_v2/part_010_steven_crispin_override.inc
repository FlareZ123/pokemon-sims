  bool issue_1022_banked_route_{false};

  bool issue_1022_items_locked_by_turn_four() const {
    // The route needs Forest Seal Stone and Mysterious Treasure on T4, so every
    // full or scheduled Item lock blocks the projection even when Items are legal on T1:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    return scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        scenario_.locks == LockMode::TurnTwoItem;
  }

  bool wonder_tag_can_bank_steven_for_known_t4_fss_route() const {
    // Field Blower removes Path to the Peak, so Wonder Tag is live once the active
    // lock state is cleared. Star Alchemy remains usable through Path itself:
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    if (state_.turn != 1 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit || rule_box_abilities_locked() ||
        issue_1022_items_locked_by_turn_four() || scenario_.max_turn < 4 ||
        !deck_seen_ || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::Oricorio || state_.retreat_used ||
        bench_space() <= 0 || !need_regi() ||
        hand_count(Card::RegidragoV) > 0 || in_play(Card::RegidragoV) ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::Arven) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Grass) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        state_.vstar_power_used ||
        !ability_available_for_pokemon(Card::RegidragoVstar)) {
      return false;
    }

    // Wonder Tag has established K1. Steven can reserve the missing Basic, Crispin,
    // and the second Grass Energy. The held opening Grass pays Oricorio's Retreat
    // Cost, Crispin plus the searched Grass establish GG on T3, and held Arven finds
    // Forest Seal Stone on T4 so Star Alchemy can supply the known VSTAR before
    // Brilliant Blender creates the strict-JIT payload:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Core turn, attachment, evolution, Tool, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
    return deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::RegidragoV) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::ForestSealStone) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
  }

  bool banked_steven_has_known_t4_fss_route() const {
    // Wonder Tag has already resolved. The remaining route is legal under a
    // removed Path, while future Item lock still blocks its T4 Tool and search Items:
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    return issue_1022_banked_route_ && state_.turn == 2 &&
        scenario_.going_first && scenario_.dci == DciProfile::StrictJit &&
        !issue_1022_items_locked_by_turn_four() && scenario_.max_turn >= 4 &&
        supporter_allowed() && deck_seen_ && state_.active &&
        state_.active->card == Card::Oricorio && state_.active->grass >= 1 &&
        !state_.manual_energy_used && !state_.retreat_used && need_regi() &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::Arven) > 0 && hand_count(Card::BrilliantBlender) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        hand_count(Card::RegidragoV) +
                deck_count_after_search_started(Card::RegidragoV) > 0 &&
        hand_count(Card::Crispin) +
                deck_count_after_search_started(Card::Crispin) > 0 &&
        hand_count(Card::Grass) +
                deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::ForestSealStone) > 0 &&
        hand_count(Card::RegidragoVstar) +
                deck_count_after_search_started(Card::RegidragoVstar) > 0;
  }

  bool play_banked_steven_for_known_t4_fss_route() {
    if (!banked_steven_has_known_t4_fss_route()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");
    std::vector<Card> found;
    for (const Card target : {Card::RegidragoV, Card::Crispin, Card::Grass}) {
      if (hand_count(target) > 0) continue;
      if (!move_deck_to_hand(target)) {
        throw std::logic_error("Known T4 Steven-FSS target disappeared");
      }
      found.push_back(target);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // These exact three targets preserve held Arven and Blender for T4 while
    // establishing the Basic and Energy package on T3:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched the T4 Steven-FSS package: " + join_cards(found) +
              ". Turn ended.");
    return true;
  }

  bool steven_should_yield_to_crispin_mysterious_route() const {
    const bool exact_lock_window = scenario_.locks == LockMode::TurnTwoItem &&
        state_.turn == 1 && !scenario_.going_first && !item_locked();
    const bool active_regi_can_attack = state_.active &&
        state_.active->card == Card::RegidragoV && !state_.manual_energy_used;
    const bool inspected_vstar_and_crispin_targets = deck_seen_ &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0;
    const bool payable_mysterious_route = hand_count(Card::MysteriousTreasure) > 0 &&
        is_legal_cost_available(false, true);

    // Wonder Tag has already established K1. Suppress Steven only when the current
    // public state proves a payable Mysterious Treasure route to Regidrago VSTAR and
    // Crispin can power the Active Regidrago V while the manual attachment remains.
    // Steven would end the turn and delay the repository's earliest-ready objective:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/774
    return exact_lock_window && active_regi_can_attack &&
        inspected_vstar_and_crispin_targets && payable_mysterious_route &&
        crispin_can_advance_energy_axis();
  }

  bool wonder_tag_can_bank_steven_for_known_t3_blender_route() const {
    // Strict JIT holds Blender for T3. No-discard-control may legally bank the
    // payload with Blender on T1, then carry that completed axis into the same
    // Steven VSTAR, Grass, and Latias continuation:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // No-discard-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1351
    const bool supported_payload_profile =
        scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::NoDiscardControl;
    if (state_.turn != 1 || !scenario_.going_first ||
        !supported_payload_profile ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !deck_seen_ || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::Fire) == 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::StevensResolve) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    const bool fresh_benched_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn <= state_.turn &&
              pokemon.grass == 0 && pokemon.fire == 0;
        });

    // Going first prevents the turn-one Supporter play, while Wonder Tag can still
    // bank Steven's Resolve. The public Grass and Fire cover the first two manual
    // attachments. Steven then takes Regidrago VSTAR, the third Grass Energy, and
    // Latias ex. Held Brilliant Blender supplies the T3 payload, and Skyliner
    // promotes the Benched attacker without consuming the T3 Supporter play:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/946
    return fresh_benched_regidrago;
  }

  bool banked_steven_has_known_t3_blender_route() const {
    // Strict JIT still needs the held Blender and a deck payload for T3. Under
    // no-discard-control, Blender may already be spent and the public discard
    // pile may already satisfy Apex Dragon before Steven resolves on T2:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // No-discard-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1351
    const bool strict_payload_route = scenario_.dci == DciProfile::StrictJit &&
        hand_count(Card::BrilliantBlender) > 0 && need_payload() &&
        payload_might_be_in_deck();
    const bool banked_no_control_payload =
        scenario_.dci == DciProfile::NoDiscardControl && payload_ready();
    if (state_.turn != 2 || !scenario_.going_first ||
        (!strict_payload_route && !banked_no_control_payload) ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::StevensResolve) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }

    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      int projected_grass = pokemon.grass;
      int projected_fire = pokemon.fire;
      int grass_in_hand = hand_count(Card::Grass);
      int fire_in_hand = hand_count(Card::Fire);
      if (!state_.manual_energy_used) {
        if (projected_grass < 2 && grass_in_hand > 0) {
          ++projected_grass;
          --grass_in_hand;
        } else if (projected_fire < 1 && fire_in_hand > 0) {
          ++projected_fire;
          --fire_in_hand;
        } else {
          continue;
        }
      }

      const int remaining_grass = std::max(0, 2 - projected_grass);
      const int remaining_fire = std::max(0, 1 - projected_fire);
      const bool searched_grass_finishes =
          remaining_grass == 1 && remaining_fire == 0;
      const bool held_fire_finishes =
          remaining_grass == 0 && remaining_fire == 1 && fire_in_hand > 0;
      if (searched_grass_finishes || held_fire_finishes) {
        // Steven's unrestricted three-card search reserves VSTAR, a direct Grass
        // Energy, and Latias. This route has no Crispin or future-draw dependency and
        // leaves the T3 Supporter play available:
        // https://api.pokemontcg.io/v2/cards/sm7-145
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/sv8-164
        // https://api.pokemontcg.io/v2/cards/sv8-76
        // https://www.pokemon.com/us/pokemon-tcg/rules
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // https://github.com/FlareZ123/pokemon-sims/issues/946
        return true;
      }
    }
    return false;
  }

  bool play_banked_steven_for_known_t3_blender_route() {
    if (!banked_steven_has_known_t3_blender_route()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> found;
    for (const Card target : {Card::RegidragoVstar, Card::Grass, Card::LatiasEx}) {
      if (!move_deck_to_hand(target)) {
        throw std::logic_error("Known banked-Steven target disappeared during legal search");
      }
      found.push_back(target);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven searches up to three cards and ends the turn. These exact K1 targets
    // establish the deterministic T3 Evolution, Energy, and promotion axes:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/issues/946
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER",
          "Searched up to 3 cards: " + join_cards(found) + ". Turn ended.");
    return true;
  }

  bool late_steven_vstar_vessel_route_available() const {
    if (!deck_seen_ || state_.turn < 2 || state_.turn >= scenario_.max_turn ||
        scenario_.dci != DciProfile::StrictJit || scenario_.locks != LockMode::None ||
        !supporter_allowed() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        !need_vstar() || !need_payload() || hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        (hand_count(Card::EarthenVessel) == 0 &&
         deck_count_after_search_started(Card::EarthenVessel) == 0) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Steven's Resolve can reserve the missing Regidrago VSTAR and Earthen Vessel,
    // then ends the current turn. On the following turn the prior-turn Active may
    // evolve, while Vessel discards the held Dragon payload and performs a real Basic
    // Energy search, satisfying the strict-JIT payload timing at the earliest known
    // ready state:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Core evolution, Item, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1017
    return deck_count_after_search_started(Card::Grass) > 0 ||
           deck_count_after_search_started(Card::Fire) > 0;
  }

  bool late_steven_active_vstar_crispin_treasure_route_available() const {
    if (!deck_seen_ || state_.turn != 2 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || item_locked() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        state_.active->grass != 0 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Grass) > 0 || hand_count(Card::Crispin) > 0 ||
        !need_energy() || !need_payload() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const int grass_in_deck = deck_count_after_search_started(Card::Grass);
    const int fire_in_deck = deck_count_after_search_started(Card::Fire);
    const int treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic));
    if (deck_count_after_search_started(Card::Crispin) == 0 ||
        grass_in_deck < 2 || fire_in_deck == 0 || treasure_targets == 0) {
      return false;
    }

    const int post_steven_deck_size = static_cast<int>(state_.deck.size()) - 2;
    const int grass_after_steven = grass_in_deck - 1;
    const int failing_topdecks = static_cast<int>(grass_after_steven == 1) +
        static_cast<int>(fire_in_deck == 1);

    // Steven searches exactly Crispin and one Grass, preserving both Basic Energy
    // types for next turn. A held Dragon then pays Mysterious Treasure while a legal
    // Psychic or Dragon target remains; Crispin attaches Grass and the normal manual
    // attachment supplies the second Grass for the Active Regidrago VSTAR's GGF cost.
    // With seed 44's K1 counts, one Fire among 42 post-search cards is the only failing
    // topdeck, so the route succeeds on 41/42 draws instead of Serena's one-card gamble:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Core Supporter, Item, attachment, search, shuffle, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1098
    return post_steven_deck_size > failing_topdecks;
  }

  std::optional<Card> late_steven_payload_treasure_bridge_payload() const {
    if (!deck_seen_ || state_.turn < 2 || state_.turn >= scenario_.max_turn ||
        scenario_.locks != LockMode::None || !supporter_allowed() || item_locked() ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        !need_payload() || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return std::nullopt;
    }

    // A held Professor Burnet is a legal current-turn Supporter outlet in this
    // exact state. It searches and discards the permitted Dragon payload before the
    // ready check, while Steven's Resolve ends the turn and delays the same axis:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, search, discard, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1117
    if (hand_count(Card::ProfessorBurnet) > 0) return std::nullopt;

    // A held Brilliant Blender is a legal current-turn deck-to-discard outlet in
    // this exact state: Active Regidrago VSTAR already pays GGF, Items are legal,
    // and payload is the only missing setup axis. Preserve the shorter one-action
    // completion instead of ending the turn for Steven's slower Treasure bridge:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, search, discard, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1111
    if (hand_count(Card::BrilliantBlender) > 0) return std::nullopt;

    const int treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic));
    if (treasure_targets < 2) return std::nullopt;

    // Steven may search a Dragon payload into hand, end the current turn, and leave
    // a second Psychic or Dragon Pokemon in deck. On the following turn the held
    // Mysterious Treasure legally discards that searched payload before finding the
    // remaining target, so the payload enters discard on the exact JIT ready turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, and attack-cost procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // JIT and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1023
    // Appletun is a Dragon Pokémon and a modeled Apex Dragon payload, so it is a
    // legal Steven target for the next-turn Mysterious Treasure discard bridge:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1258
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::DialgaGX, Card::GoodraVstar,
                               Card::Appletun}) {
      if (deck_count_after_search_started(payload) > 0) return payload;
    }
    return std::nullopt;
  }

  Card choose_supporter_after_search_started() {

if (wonder_tag_can_bank_steven_for_known_t3_active_regi_route()) {
  // Prefer the complete Steven package over Crispin, which powers the
  // Basic but leaves both Evolution and strict-JIT payload unresolved:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/issues/1049
  return Card::StevensResolve;
}
    if (wonder_tag_can_bank_steven_for_known_t4_fss_route()) {
      issue_1022_banked_route_ = true;
      // Wonder Tag may bank a Supporter on the first player's opening turn even
      // though that Supporter cannot be played until T2. Select the complete Steven
      // package instead of Crispin, whose Energy effect is unusable while the Basic
      // Regidrago axis remains unresolved:
      // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
      return Card::StevensResolve;
    }
    if (wonder_tag_can_bank_steven_for_known_t3_blender_route() ||
        (late_steven_vstar_vessel_route_available() &&
         deck_count_after_search_started(Card::StevensResolve) > 0)) {
      // Wonder Tag searches a Supporter after legal deck inspection. Prefer Steven's
      // complete next-turn VSTAR-Vessel route over a Burnet target that cannot repair
      // the unresolved Evolution axis:
      // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1017
      return Card::StevensResolve;
    }
    return static_cast<const Engine&>(*this).choose_supporter_after_search_started();
  }

  bool late_steven_vstar_grass_with_held_blender_k0_route_available() const {
    if (deck_seen_ || state_.turn >= scenario_.max_turn ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::Grass) > 0 || !need_vstar() || !need_energy() ||
        !need_payload() || !payload_might_be_in_deck()) {
      return false;
    }

    // At K0, fixed-list VSTAR and Grass targets remain plausible until Steven's
    // legal search inspects the deck. Steven may reserve both and end the turn;
    // next turn the prior-turn Active can attach, evolve, and use held Brilliant
    // Blender for the same-turn strict-JIT payload without another Supporter:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core attachment, evolution, Supporter, Item, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0 hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
   // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1113
    return grass_needed() == 1 && fire_needed() == 0 &&
        might_be_unseen(Card::RegidragoVstar) && might_be_unseen(Card::Grass);
  }

  bool late_steven_vstar_grass_with_held_burnet_route_available() const {
    if (!deck_seen_ || state_.turn >= scenario_.max_turn ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::BrilliantBlender) > 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::Grass) > 0 || !need_vstar() || !need_energy() ||
        !need_payload() || !payload_might_be_in_deck()) {
      return false;
    }

    // Steven's Resolve can reserve the known Regidrago VSTAR and final Grass
    // Energy, then end the turn. On the next turn the prior-turn Active may take
    // the searched manual attachment, evolve, and use held Professor Burnet to
    // discard a permitted Dragon payload during that same strict-JIT ready turn:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Grass Energy and core attachment, evolution, Supporter, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1112
    return grass_needed() == 1 && fire_needed() == 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0;
  }

  bool late_steven_vstar_with_held_blender_route_available() const {
    if (!deck_seen_ || state_.turn >= scenario_.max_turn ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        !need_vstar() || !need_payload() || !payload_might_be_in_deck()) {
      return false;
    }

    // Steven's Resolve may reserve the known-searchable Regidrago VSTAR and end
    // the turn. On the following turn the prior-turn Active may evolve before the
    // held Brilliant Blender searches and discards a permitted Dragon payload on
    // that same strict-JIT ready turn. This complete route outranks Gladion for a
    // backup Basic because the Active-position and GGF Energy axes are already met:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Core evolution, Supporter, Item, search, discard, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1109
    return deck_count_after_search_started(Card::RegidragoVstar) > 0;
  }

  bool late_steven_has_active_regidrago_t3_blender_route() const {
    if (state_.turn != 2 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        !state_.manual_energy_used || state_.active->grass < 2 ||
        state_.active->fire != 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::BrilliantBlender) > 0 ||
        !need_vstar() || !need_payload() || !payload_might_be_in_deck() ||
        !might_be_unseen(Card::RegidragoVstar) ||
        !might_be_unseen(Card::BrilliantBlender)) {
      return false;
    }

    // Steven is selected at K0 before any expendable search Item is burned. Fixed-list
    // targets may remain plausible until Steven legally inspects the deck; its search
    // then chooses only targets actually present. A current-turn VSTAR connector still
    // outranks this banked route. Otherwise the held Fire completes GGF on T3 and the
    // prior-turn Active may evolve before Brilliant Blender supplies the JIT payload:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/952
    return !blender_vstar_card_axis_can_finish_this_turn();
  }

  bool should_play_steven() const {
    const bool wonder_tag_admission_remains_productive =
        wonder_tag_selected_steven_this_turn_ && state_.turn == 1 &&
        !scenario_.going_first && supporter_allowed() &&
        hand_count(Card::StevensResolve) > 0 && setup_axis_missing() &&
        !steven_should_yield_to_crispin_mysterious_route();

    // Wonder Tag legally chose Steven before later Item and Bench actions changed the
    // mutable axis count. Steven still banks up to three future setup cards and ends a
    // turn that cannot produce a ready Regidrago VSTAR because a newly played Basic
    // cannot evolve that turn. Preserve that admission unless the proven #774 faster
    // current route is now live or every setup axis is already complete:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR evolution: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/999
    return (should_play_steven_original() &&
            !steven_should_yield_to_crispin_mysterious_route()) ||
        wonder_tag_admission_remains_productive ||
        late_steven_is_only_basic_connector() ||
        late_steven_has_known_t3_compression_route() ||
        banked_steven_has_known_t3_blender_route() ||
        late_steven_vstar_grass_with_held_blender_k0_route_available() ||
        late_steven_vstar_grass_with_held_burnet_route_available() ||
        late_steven_vstar_with_held_blender_route_available() ||

(state_.turn == 2 && scenario_.going_first &&
 scenario_.dci == DciProfile::MatchupFlexJit &&
 scenario_.locks == LockMode::None && supporter_allowed() &&
 state_.active && state_.active->card == Card::RegidragoV &&
 state_.active->entered_turn < state_.turn &&
 hand_count(Card::StevensResolve) > 0 && need_vstar() &&
 need_payload() && payload_might_be_in_deck() &&
 deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
 deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
 ((grass_needed() == 1 && fire_needed() == 0 && hand_count(Card::Grass) > 0) ||
  (grass_needed() == 0 && fire_needed() == 1 && hand_count(Card::Fire) > 0))) ||
        late_steven_has_active_regidrago_t3_blender_route() ||
        late_steven_vstar_vessel_route_available() ||
        late_steven_active_vstar_crispin_treasure_route_available() ||
        late_steven_payload_treasure_bridge_payload().has_value();
  }

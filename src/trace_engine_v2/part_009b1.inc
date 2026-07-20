    return true;
  }

  std::optional<Card> quick_ball_strict_deadlock_fallback_cost() const {
    if (scenario_.dci != DciProfile::StrictJit) return std::nullopt;

    // Channeler removes effects of attacks from the player and their Pokémon. The
    // setup simulator executes no opposing attacks, so Channeler has no present
    // modeled setup effect and is the lowest-DCI singleton in this exact deadlock:
    // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
    // Opponent-action boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
    // DCI approximation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1018
    if (hand_count(Card::Channeler) > 0) return Card::Channeler;

    const bool team_yell_has_recovery_target = std::any_of(
        state_.discard.begin(), state_.discard.end(), [](const Card card) {
          return card != Card::TeamYellsCheer && (is_pokemon(card) || is_supporter(card));
        });
    // Team Yell's Cheer can recover only Pokémon and Supporters already in discard.
    // With no eligible target, preserving the singleton while passing the missing
    // Basic axis has less present setup value than paying Quick Ball's legal cost:
    // Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Earliest-ready priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1018
    if (!team_yell_has_recovery_target && hand_count(Card::TeamYellsCheer) > 0) {
      return Card::TeamYellsCheer;
    }
    return std::nullopt;
  }

  std::optional<Card> quick_ball_crispin_replaced_energy_cost() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 1 ||
        scenario_.locks != LockMode::None || !supporter_allowed() ||
        state_.manual_energy_used || state_.supporter_used ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::Grass) < 2 ||
        hand_count(Card::Arven) == 0 || hand_count(Card::BrilliantBlender) == 0 ||
        state_.vstar_power_used || !can_free_retreat_with_latias()) {
      return std::nullopt;
    }

    const bool grass_available = deck_seen_
        ? deck_count_after_search_started(Card::Grass) > 0
        : might_be_unseen(Card::Grass);
    const bool fire_available = deck_seen_
        ? deck_count_after_search_started(Card::Fire) > 0
        : might_be_unseen(Card::Fire);
    const bool fss_available = hand_count(Card::ForestSealStone) > 0 ||
        (deck_seen_ ? deck_count_after_search_started(Card::ForestSealStone) > 0
                    : might_be_unseen(Card::ForestSealStone));
    const bool vstar_available = hand_count(Card::RegidragoVstar) > 0 ||
        (deck_seen_ ? deck_count_after_search_started(Card::RegidragoVstar) > 0
                    : might_be_unseen(Card::RegidragoVstar));
    if (!grass_available || !fire_available || !fss_available ||
        !vstar_available || !payload_might_be_in_deck()) {
      return std::nullopt;
    }

    // Quick Ball may discard one held Grass because the other Grass supplies the
    // turn-one manual attachment and held Crispin immediately searches Grass plus
    // Fire, attaches the replacement Grass, and leaves Fire for turn two. Held Arven
    // then finds Forest Seal Stone, whose Star Alchemy can find Regidrago VSTAR;
    // Brilliant Blender supplies the turn-two strict-JIT payload, and Latias ex
    // supplies the legal zero-Retreat promotion. This fallback runs only after
    // ordinary lower-DCI costs fail and only for the exact missing-Regidrago-V route:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official Item, Supporter, Energy, evolution, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1093
    return Card::Grass;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::QuickBall) == 0) return false;
    // A strict-JIT Quick Ball payload cost is useful only when the Regidrago VSTAR
    // card axis can finish in the same turn. Quick Ball searches only Basic Pokémon,
    // so it cannot repair a missing VSTAR card after spending the held Dragon:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/953
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                  blender_vstar_card_axis_can_finish_this_turn() &&
                                  std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Quick Ball searches for a Basic Pokémon. At K0, a target remains plausible only
    // when a fixed-list Basic copy is not accounted for in public zones. At K1, the
    // inspected deck is checked exactly before paying the discard requirement:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/694
    const bool search_has_target = deck_seen_
        ? std::any_of(state_.deck.begin(), state_.deck.end(), is_basic)
        : std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
            return is_basic(entry.first) && might_be_unseen(entry.first);
          });
    if (!search_has_target) return false;
    // Quick Ball searches only the deck after paying its discard cost. K0 keeps
    // fixed-list Basic targets plausible; K1 requires the inspected deck to contain
    // the desired Basic before spending both cards:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool want_regi = need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV);
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
                             might_be_unseen(Card::LatiasEx);
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 &&
                           needs_tapu_connector() && might_be_unseen(Card::TapuLeleGX);
    const bool want_oricorio = hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0 &&
                                  needs_oricorio_connector() && might_be_unseen(Card::Oricorio);
    // Quick Ball searches only a Basic Pokémon. Dialga-GX is the only modeled
    // Dragon payload that is Basic; Mega Dragonite ex and Dragapult ex are Stage 2,
    // and Hisuian Goodra VSTAR is a VSTAR Evolution Pokémon:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh11-136
    // Quick Ball pays its discard before the fetched Basic reaches hand. A later
    // Ultra Ball route is live only when its own two-card cost remains payable:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    const bool want_payload = need_payload() && might_be_unseen(Card::DialgaGX) && !can_jit_discard &&
                              has_payable_payload_outlet_after_costed_search(Card::QuickBall);
    if (!can_jit_discard && !want_regi && !want_latias && !want_tapu && !want_oricorio && !want_payload) return false;
    auto cost = choose_discard(can_jit_discard, true, true, Card::QuickBall);
    if (!cost && want_regi && hand_count(Card::RegidragoVstar) >= 2) {
      // One of two public VSTAR copies may pay Quick Ball only while Quick Ball is
      // establishing the missing Regidrago V. The other copy remains in hand for the
      // legal later evolution, so this route-conditioned fallback does not weaken the
      // singleton VSTAR protection or alter ordinary lower-DCI selector ordering:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/964
      cost = Card::RegidragoVstar;
    }
    if (!cost && want_regi) {
      cost = quick_ball_crispin_replaced_energy_cost();
    }
    if (!cost && want_regi) {
      // This fallback is reached only after every ordinary strict-DCI cost category
      // and the duplicate-VSTAR route failed. It is limited to the exact missing
      // Regidrago V axis and never exposes a Dragon payload or live singleton route:
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Core Item, search, and Bench procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1018
      cost = quick_ball_strict_deadlock_fallback_cost();
    }
    if (!cost) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(*cost, "Quick Ball cost", "R-QB-01");
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh1-179
    record_deck_search_knowledge("Quick Ball");
    const std::array<Card, 6> targets{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::DialgaGX, Card::MawileGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (target == Card::Oricorio && want_oricorio) ||

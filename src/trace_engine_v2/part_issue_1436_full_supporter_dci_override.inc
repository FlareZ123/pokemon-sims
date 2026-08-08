  int issue_1419_publicly_possible_count(const Card card) const {
    if (deck_seen_ || prizes_revealed_) {
      return deck_count_after_search_started(card);
    }
    const int known = hand_count(card) + count_of(state_.discard, card) +
        in_play_count(card) + attached_public_count(card);
    return std::max(0, total_copies(card) - known);
  }

  int issue_1419_possible_payload_count() const {
    int possible_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      possible_payloads += issue_1419_publicly_possible_count(card);
    }
    return possible_payloads;
  }

  bool quick_ball_oricorio_latias_route_available() const {
    // This route may spend protected singleton Latias ex and suppress Wonder Tag
    // only after a legal deck or Prize inspection proves singleton Oricorio is in
    // the deck. K0 public accounting cannot distinguish deck from Prize cards:
    // Quick Ball cost-before-search text: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Core Item, search, Bench, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1467
    if (!deck_seen_ && !prizes_revealed_) return false;

    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    const int missing_energy = missing_grass + missing_fire;
    const int future_turns = std::max(0, scenario_.max_turn - state_.turn);
    const int attachment_windows = future_turns +
        (state_.manual_energy_used ? 0 : 1);

    // The shared route gate controls both Quick Ball's dynamic DCI cost and its
    // Basic-target priority. Keeping one predicate prevents Wonder Tag into Crispin
    // from displacing the stronger Vital Dance line after Latias ex becomes inert:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Core Item, Bench, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/1419
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && active_is_vstar() &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio) &&
        might_be_unseen(Card::Oricorio) && missing_energy > 0 &&
        missing_energy <= 2 && attachment_windows >= missing_energy &&
        issue_1419_publicly_possible_count(Card::Grass) >= missing_grass &&
        issue_1419_publicly_possible_count(Card::Fire) >= missing_fire &&
        hand_count(Card::BrilliantBlender) > 0 && !payload_ready() &&
        issue_1419_possible_payload_count() > future_turns;
  }

  bool issue_1419_oricorio_route_should_hold_supporter() const {
    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    const int missing_energy = missing_grass + missing_fire;
    const int future_turns = std::max(0, scenario_.max_turn - state_.turn);
    const int attachment_windows = future_turns +
        (state_.manual_energy_used ? 0 : 1);

    // After Vital Dance has put the exact missing Energy into hand, a draw-mode
    // Supporter can only damage the proven route by shuffling away those Energy or
    // Brilliant Blender. Preserve the hand until the scheduled attachments finish:
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tate & Liza draw mode: https://api.pokemontcg.io/v2/cards/sm7-148
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // One-Supporter-per-turn and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/1419
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && active_is_vstar() &&
        in_play(Card::Oricorio) && missing_energy > 0 &&
        hand_count(Card::Grass) >= missing_grass &&
        hand_count(Card::Fire) >= missing_fire &&
        attachment_windows >= missing_energy &&
        hand_count(Card::BrilliantBlender) > 0 && !payload_ready() &&
        issue_1419_possible_payload_count() > future_turns;
  }

  bool issue_1476_redundant_burnet_route_available(
      const std::optional<Card> excluded_from_cost) const {
    if (!excluded_from_cost || *excluded_from_cost != Card::MysteriousTreasure) {
      return false;
    }

    // Held Brilliant Blender strictly covers Professor Burnet's remaining modeled
    // deck-payload role. The public T1 hand may therefore spend Burnet to establish
    // K1 through Mysterious Treasure, then use the inspected Oricorio, Quick Ball,
    // Tapu Lele-GX, and Crispin line to complete GGF on T2. Every irreversible cost
    // precedes the first legal inspection, so this gate uses only fixed-list K0
    // possibility accounting and preserves two required Bench slots. A two-turn
    // horizon is sufficient because the route reaches readiness on T2:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core first-turn, Item, Supporter, Ability, Bench, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn == 1 &&
        scenario_.going_first && scenario_.max_turn >= 2 && !item_locked() &&
        state_.active && state_.active->card == Card::RegidragoV &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 && !payload_ready() &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        hand_count(Card::Crispin) == 0 &&
        hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
        bench_space() >= 2 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::Oricorio) &&
        issue_1419_publicly_possible_count(Card::TapuLeleGX) >= 1 &&
        issue_1419_publicly_possible_count(Card::Crispin) >= 1 &&
        issue_1419_publicly_possible_count(Card::Oricorio) >= 1 &&
        issue_1419_publicly_possible_count(Card::Grass) >= 2 &&
        issue_1419_publicly_possible_count(Card::Fire) >= 1 &&
        issue_1419_possible_payload_count() > 0;
  }

  std::optional<Card> issue_1765_treasure_surplus_energy_cost(
      const std::optional<Card> excluded_from_cost) const {
    if (excluded_from_cost != Card::MysteriousTreasure ||
        scenario_.dci != DciProfile::StrictJit || item_locked() ||
        state_.turn != 2 || scenario_.max_turn < 3 ||
        state_.manual_energy_used || state_.vstar_power_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn || !need_vstar() ||
        !need_payload() || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        !might_be_unseen(Card::RegidragoVstar)) {
      return std::nullopt;
    }

    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    const int attachment_windows = 1 + scenario_.max_turn - state_.turn;
    if (missing_grass + missing_fire == 0 ||
        attachment_windows < missing_grass + missing_fire ||
        hand_count(Card::Grass) < missing_grass ||
        hand_count(Card::Fire) < missing_fire) {
      return std::nullopt;
    }

    // Mysterious Treasure may spend only the Basic Energy copy left after reserving
    // every type and count required by the earliest observable manual-attachment
    // schedule. The prior-turn Regidrago V is evolution-eligible, Treasure can search
    // Regidrago VSTAR, and the retained Grass plus Fire complete GGF across T2 and T3.
    // Earthen Vessel remains as the next-turn one-discard outlet without weakening
    // the general strict-DCI protection applied to Energy in all other states:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Item costs, manual attachments, evolution, and VSTAR Power procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug and source-bound CI reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1765 https://github.com/FlareZ123/pokemon-sims/pull/1768
    if (hand_count(Card::Grass) > missing_grass) return Card::Grass;
    return std::nullopt;
  }

  std::optional<Card> issue_2264_latias_burnet_replaced_supporter_cost(
      const std::optional<Card> excluded_from_cost) const {
    const bool one_discard_latias_search =
        excluded_from_cost == Card::MysteriousTreasure ||
        excluded_from_cost == Card::QuickBall;
    const bool jit_profile = scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::MatchupFlexJit;
    const bool latias_known_in_deck = prizes_known() &&
        deck_count_after_search_started(Card::LatiasEx) > 0;
    const bool burnet_payload_route = hand_count(Card::ProfessorBurnet) > 0 &&
        supporter_allowed() && !state_.supporter_used &&
        can_play_payload_this_turn() &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);

    if (!one_discard_latias_search || !jit_profile ||
        scenario_.locks != LockMode::None || item_locked() ||
        !prizes_known() || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || !need_active_vstar() ||
        need_energy() || !benched_vstar_promotion_ready() ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        !latias_known_in_deck || !burnet_payload_route) {
      return std::nullopt;
    }

    // Mysterious Treasure and Quick Ball each discard exactly one other card before
    // searching. In this exact K1 route they search Latias ex, Skyliner gives the
    // Basic Active no Retreat Cost, and held Professor Burnet supplies the required
    // same-turn Dragon payload. Once the selected Benched VSTAR already has GGF,
    // Crispin advances no remaining setup axis. Steven's Resolve also loses its
    // setup role because it ends the turn while Burnet completes the route now.
    // Ordinary lower-DCI costs are still preferred because this helper runs only
    // after the generic and existing route-specific discard selectors fail:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2264
    if (hand_count(Card::Crispin) > 0) return Card::Crispin;
    if (hand_count(Card::StevensResolve) > 0) return Card::StevensResolve;
    return std::nullopt;
  }

  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard_issue1436_original(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    if (ordinary) return ordinary;

    if (const auto surplus_energy =
            issue_1765_treasure_surplus_energy_cost(excluded_from_cost)) {
      return surplus_energy;
    }

    if (issue_1476_redundant_burnet_route_available(excluded_from_cost)) {
      // Blender dominates Burnet's only remaining payload axis, while spending
      // Burnet unlocks the confirmed Oricorio -> Tapu Lele-GX -> Crispin route:
      // https://github.com/FlareZ123/pokemon-sims/issues/1476
      // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sm2-55
      return Card::ProfessorBurnet;
    }

    const int public_regidrago_lines =
        in_play_count(Card::RegidragoV) + in_play_count(Card::RegidragoVstar);
    if (hand_count(Card::RegidragoV) > 0 && public_regidrago_lines >= 2) {
      // A public Regidrago VSTAR stack contains the Regidrago V it evolved from.
      // Once that evolved line and a second public Regidrago line already exist,
      // a third held Basic is redundant for the modeled setup objective and may
      // pay a legal discard cost under dynamic DCI:
      // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Core evolution and public-board procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1473
      return Card::RegidragoV;
    }

    if (excluded_from_cost && *excluded_from_cost == Card::QuickBall &&
        quick_ball_oricorio_latias_route_available()) {
      // Active Regidrago VSTAR has already replaced Latias ex's setup role. The
      // shared gate proves that Oricorio, future attachments, and Blender complete
      // every remaining axis before this singleton becomes legal Quick Ball fuel:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/sm2-55
      // https://github.com/FlareZ123/pokemon-sims/issues/1419
      return Card::LatiasEx;
    }

    const bool issue_1576_vessel_finish =
        excluded_from_cost == Card::EarthenVessel &&
        scenario_.dci == DciProfile::NoDiscardControl &&
        scenario_.locks == LockMode::None && state_.turn == 2 &&
        deck_seen_ && state_.supporter_used && !state_.manual_energy_used &&
        active_is_vstar() && grass_needed() == 1 && fire_needed() == 0 &&
        need_payload() && hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        std::any_of(state_.deck.begin(), state_.deck.end(),
                    [](const Card card) {
                      return is_payload(card) &&
                          is_dragon_or_psychic(card);
                    });
    if (issue_1576_vessel_finish) {
      // Crispin has consumed the Supporter play, so Steven cannot resolve
      // this turn. Spending it through Vessel unlocks the deterministic T2
      // finish and dominates preserving it for a later setup turn:
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // One-Supporter and Item cost procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1576
      return Card::StevensResolve;
    }

    const bool issue_1576_treasure_finish =
        excluded_from_cost == Card::MysteriousTreasure &&
        scenario_.dci == DciProfile::NoDiscardControl &&
        scenario_.locks == LockMode::None && state_.turn == 2 &&
        deck_seen_ && state_.supporter_used && !state_.manual_energy_used &&
        active_is_vstar() && grass_needed() == 1 && fire_needed() == 0 &&
        need_payload() && hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::Grass) >= 2 &&
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::StevensResolve) > 0;
    if (issue_1576_treasure_finish) {
      // Vessel searched two Grass Energy. One pays Treasure while the other
      // remains for the unused manual attachment; Quick Ball then discards
      // the searched Dragon payload:
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1576
      return Card::Grass;
    }

    if (const auto replaced_supporter =
            issue_2264_latias_burnet_replaced_supporter_cost(excluded_from_cost)) {
      return replaced_supporter;
    }

    if (scenario_.locks != LockMode::FullSupporter) return std::nullopt;

    // Full Supporter lock disables every Supporter effect for the complete
    // modeled horizon. A held Supporter therefore has zero Supporter AMR and may
    // pay an Item's printed discard cost after every ordinary lower-DCI option
    // has been exhausted. The card being played remains excluded from its own
    // cost, and every non-Supporter singleton keeps its existing protection:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1436
    for (const Card card : state_.hand) {
      if (!is_supporter(card)) continue;
      const int excluded = excluded_from_cost && *excluded_from_cost == card ? 1 : 0;
      if (hand_count(card) > excluded) return card;
    }
    return std::nullopt;
  }
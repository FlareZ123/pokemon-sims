  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0) return false;

    const bool known_no_basic_prize = prizes_known() &&
        std::none_of(state_.prizes.begin(), state_.prizes.end(), is_basic);
    if (known_no_basic_prize) {
      // Hisuian Heavy Ball remains a legal held discard-cost resource after K1 proves
      // that no Basic Pokémon is in the Prize cards. Stabilization may re-evaluate this
      // unchanged no-op state, so expose one decision per exact state:
      // https://api.pokemontcg.io/v2/cards/swsh10-146
      // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
      // https://github.com/FlareZ123/pokemon-sims/issues/1007
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1007-heavy-ball-known-no-basic", state_.turn,
            "HOLD ITEM", "R-HEAVYBALL-01; P-KNOWLEDGE-01",
            "Known Prize cards contain no Basic Pokemon; retained Heavy Ball as a legal discard cost.");
      }
      return false;
    }

    // Card text source: https://api.pokemontcg.io/v2/cards/swsh10-146
    // At K0, Heavy Ball is an information action as well as a possible recovery.
    // It is played before ordinary search Items. After inspection, any revealed
    // Basic can be selected for its immediate connector value or as future fodder.
    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;

    const auto prized = [this](const Card card) {
      return std::find(state_.prizes.begin(), state_.prizes.end(), card) != state_.prizes.end();
    };
    const bool can_bench_recovered_basic = bench_space() > 0;
    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool required_energy_still_in_deck =
        (grass_needed() == 1 && might_be_unseen(Card::Grass)) ||
        (fire_needed() == 1 && might_be_unseen(Card::Fire));
    const bool payload_still_in_deck = might_be_unseen(Card::MegaDragonite) ||
                                         might_be_unseen(Card::Dragapult) ||
                                         might_be_unseen(Card::GoodraVstar) ||
                                         might_be_unseen(Card::DialgaGX);
    // P-COMPRESS-01: with one missing Energy and Professor Burnet as the live
    // current-turn payload Supporter, Oricorio plus a manual attachment preserves
    // the Supporter slot. Tapu Lele-GX -> Crispin would consume it instead.
    // Oricorio source: https://api.pokemontcg.io/v2/cards/sm2-55
    // Professor Burnet source: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool oricorio_preserves_burnet = can_bench_recovered_basic && need_energy() && exactly_one_energy_missing &&
                                           !state_.manual_energy_used && can_play_payload_this_turn() &&
                                           hand_count(Card::ProfessorBurnet) > 0 && hand_count(Card::BrilliantBlender) == 0 &&
                                           required_energy_still_in_deck && payload_still_in_deck;
    Card selected = Card::HisuianHeavyBall;

    // Direct current-turn axes first. A recovered Basic cannot be an immediate
    // connector when the Bench is full, although it remains valid future fodder.
    if (need_regi() && can_bench_recovered_basic && prized(Card::RegidragoV)) selected = Card::RegidragoV;
    else if (oricorio_preserves_burnet && prized(Card::Oricorio)) selected = Card::Oricorio;
    else if (can_bench_recovered_basic && needs_tapu_connector() && prized(Card::TapuLeleGX)) selected = Card::TapuLeleGX;
    else if (can_bench_recovered_basic && needs_oricorio_connector() && prized(Card::Oricorio)) selected = Card::Oricorio;
    else if (can_bench_recovered_basic && need_active_vstar() && ability_available_for_pokemon(Card::LatiasEx) && prized(Card::LatiasEx)) selected = Card::LatiasEx;

    // Dialga-GX is a Basic Dragon payload. When payload is the only missing axis,
    // prefer it before future connectors if a surviving one-discard Item can put it
    // into discard this turn:
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool payload_is_only_missing_axis = need_payload() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_active_vstar();
    const bool live_one_discard_payload_outlet = !state_.deck.empty() &&
        (hand_count(Card::EarthenVessel) > 0 || hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0);
    if (selected == Card::HisuianHeavyBall && payload_is_only_missing_axis &&
        can_play_payload_this_turn() && live_one_discard_payload_outlet && prized(Card::DialgaGX)) {
      selected = Card::DialgaGX;
    }

    // Prefer future connectors whose abilities can function in this scenario.
    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::Oricorio, Card::TapuLeleGX,
                                   Card::LatiasEx, Card::DialgaGX, Card::MawileGX}) {
        if ((candidate == Card::TapuLeleGX || candidate == Card::LatiasEx) &&
            !ability_available_for_pokemon(candidate)) continue;
        if (prized(candidate)) { selected = candidate; break; }
      }
    }

    // If all functional connector choices are absent, still take any Basic as
    // future legal discard fodder rather than leave it prized.
    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::TapuLeleGX, Card::Oricorio,
                                   Card::LatiasEx, Card::DialgaGX, Card::MawileGX}) {
        if (prized(candidate)) { selected = candidate; break; }
      }
    }

    if (selected != Card::HisuianHeavyBall) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), selected);
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
            "Looked at Prize cards and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }

    // If no Basic was present, Heavy Ball resolves normally and is discarded.
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          "Looked at Prize cards; no Basic Pokemon was present, so Heavy Ball was discarded.");
    return true;
  }

  bool need_regi() const { return !has_any_regi(); }
  // The VSTAR card axis is complete once the card is in hand; evolve_best_regi
  // resolves the later game-state transition when its timing condition is legal.
  bool need_vstar() const { return has_regi_v() && !has_vstar() && hand_count(Card::RegidragoVstar) == 0; }

  bool has_other_payload_outlet(const Card excluded) const {
    // A search connector can only fetch a future payload when a payload may still
    // be in deck. K1 derives this from legal deck or Prize inspection.
    if (!payload_might_be_in_deck()) return false;
    // The fetched payload is in hand, so the continuation must discard from hand.
    // Mysterious Treasure, Quick Ball, Ultra Ball, and Earthen Vessel have hand-discard
    // costs: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/sv4-163
    // Serena can discard from hand only while the turn's Supporter play remains available:
    // https://api.pokemontcg.io/v2/cards/swsh12-164 https://www.pokemon.com/us/pokemon-tcg/rules
    // Evolution Incense has no discard effect, while Brilliant Blender and Professor Burnet
    // discard cards selected from deck, so none can dispose of this fetched hand payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const auto present = [this, excluded](const Card card) {
      return hand_count(card) > (card == excluded ? 1 : 0);
    };
    // The current search Item and its mandatory discard resolve before the fetched
    // payload reaches hand. A claimed continuation must survive both operations;
    // Mysterious Treasure and Quick Ball both require that upfront discard:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179
    const auto preserves_hand_discard_outlet = [this, excluded](const Card outlet) {
      const auto cost = choose_discard(false, true);
      if (!cost) return false;
      const int played_copy = outlet == excluded ? 1 : 0;
      const int cost_copy = *cost == outlet ? 1 : 0;
      return hand_count(outlet) > played_copy + cost_copy;
    };
    // The current Treasure or Quick Ball line spends its search card and one discard,
    // then adds the fetched payload. Ultra Ball still needs two other cards when played:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Its post-search hand therefore needs Ultra Ball, the fetched payload, and one
    // further card, which requires at least four cards in the pre-search hand.
    const bool payable_ultra_ball = present(Card::UltraBall) && state_.hand.size() >= 4U;
    return preserves_hand_discard_outlet(Card::MysteriousTreasure) ||
           preserves_hand_discard_outlet(Card::QuickBall) || payable_ultra_ball ||
           preserves_hand_discard_outlet(Card::EarthenVessel) ||
           (supporter_allowed() && present(Card::Serena));
  }

  Card fallback_mysterious_target_after_search_started() const {
    // Mysterious Treasure can search a Psychic or Dragon Pokémon: https://api.pokemontcg.io/v2/cards/sm6-113
    // Appletun sv8-140 and Dipplin TWM 127 are Dragon Pokémon and remain legal
    // fallbacks. Dipplin's legal-search status stays separate from is_payload:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sv6-127
    // https://github.com/FlareZ123/pokemon-sims/issues/1253
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::Appletun, Card::Dipplin}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  Card fallback_quick_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX,
                              Card::MawileGX, Card::DialgaGX}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
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
    const Pokemon* energy_target = target_regi();
    const bool required_energy_still_in_deck = energy_target != nullptr &&
        completing_basic_energy_for(*energy_target, [this](const Card card) {
          return might_be_unseen(card);
        }).has_value();
    const bool exactly_one_energy_missing = required_energy_still_in_deck;
    // DDE-only exposes Grass and Fire as alternative one-card completions, not
    // additive deficits: https://github.com/FlareZ123/pokemon-sims/issues/2368
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
    else if (can_bench_recovered_basic && !state_.dark_asset_used && state_.hand.size() <= 5U &&
             ability_available_for_pokemon(Card::CrobatV) && prized(Card::CrobatV)) selected = Card::CrobatV;
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
                                   Card::LatiasEx, Card::CrobatV, Card::DialgaGX, Card::MawileGX}) {
        if ((candidate == Card::TapuLeleGX || candidate == Card::LatiasEx ||
             candidate == Card::CrobatV) && !ability_available_for_pokemon(candidate)) continue;
        if (prized(candidate)) { selected = candidate; break; }
      }
    }

    // If all functional connector choices are absent, still take any Basic as
    // future legal discard fodder rather than leave it prized.
    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::TapuLeleGX, Card::Oricorio,
                                   Card::LatiasEx, Card::CrobatV, Card::DialgaGX, Card::MawileGX}) {
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

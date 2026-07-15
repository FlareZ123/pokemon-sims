  bool prize_payload_one_discard_outlet_has_legal_target() const {
    // These Items can discard a recovered Dragon payload only when their mandatory
    // search has a legal target in the inspected deck. Heavy Ball establishes K1
    // before choosing a Prize card, while the Gladion route below already requires
    // known Prize cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool mysterious_target = hand_count(Card::MysteriousTreasure) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    const bool quick_target = hand_count(Card::QuickBall) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);
    const bool vessel_target = hand_count(Card::EarthenVessel) > 0 &&
        (deck_count_after_search_started(Card::Grass) > 0 ||
         deck_count_after_search_started(Card::Fire) > 0);
    return !item_locked() && (mysterious_target || quick_target || vessel_target);
  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0) return false;

    const bool known_no_basic_prize = prizes_known() &&
        std::none_of(state_.prizes.begin(), state_.prizes.end(), is_basic);
    if (known_no_basic_prize) {
      trace("HOLD ITEM", "R-HEAVYBALL-01; P-KNOWLEDGE-01",
            "Known Prize cards contain no Basic Pokemon; retained Heavy Ball as a legal discard cost.");
      return false;
    }

    // Hisuian Heavy Ball reveals the Prize cards before its optional Basic recovery.
    // The revealed information therefore permits an exact K1 continuation check:
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
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
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool oricorio_preserves_burnet = can_bench_recovered_basic && need_energy() &&
        exactly_one_energy_missing && !state_.manual_energy_used &&
        can_play_payload_this_turn() && hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::BrilliantBlender) == 0 && required_energy_still_in_deck &&
        payload_still_in_deck;
    Card selected = Card::HisuianHeavyBall;

    if (need_regi() && can_bench_recovered_basic && prized(Card::RegidragoV)) {
      selected = Card::RegidragoV;
    } else if (oricorio_preserves_burnet && prized(Card::Oricorio)) {
      selected = Card::Oricorio;
    } else if (can_bench_recovered_basic && needs_tapu_connector() && prized(Card::TapuLeleGX)) {
      selected = Card::TapuLeleGX;
    } else if (can_bench_recovered_basic && needs_oricorio_connector() && prized(Card::Oricorio)) {
      selected = Card::Oricorio;
    } else if (can_bench_recovered_basic && need_active_vstar() &&
               ability_available_for_pokemon(Card::LatiasEx) && prized(Card::LatiasEx)) {
      selected = Card::LatiasEx;
    }

    const bool payload_is_only_missing_axis = need_payload() && !need_regi() &&
        !need_vstar() && !need_energy() && !need_active_vstar();
    const bool live_payload_outlet = prize_payload_one_discard_outlet_has_legal_target();
    // Dialga-GX is useful as an immediate Apex Dragon payload only when the held
    // one-discard Item can legally resolve after the Prize exchange:
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    if (selected == Card::HisuianHeavyBall && payload_is_only_missing_axis &&
        can_play_payload_this_turn() && live_payload_outlet && prized(Card::DialgaGX)) {
      selected = Card::DialgaGX;
    }

    const bool avoid_stranded_dialga = payload_is_only_missing_axis && !live_payload_outlet;
    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::Oricorio, Card::TapuLeleGX,
                                   Card::LatiasEx, Card::DialgaGX, Card::MawileGX}) {
        if (candidate == Card::DialgaGX && avoid_stranded_dialga) continue;
        if ((candidate == Card::TapuLeleGX || candidate == Card::LatiasEx) &&
            !ability_available_for_pokemon(candidate)) continue;
        if (prized(candidate)) {
          selected = candidate;
          break;
        }
      }
    }

    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::TapuLeleGX, Card::Oricorio,
                                   Card::LatiasEx, Card::DialgaGX, Card::MawileGX}) {
        if (candidate == Card::DialgaGX && avoid_stranded_dialga) continue;
        if (prized(candidate)) {
          selected = candidate;
          break;
        }
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

    // The printed effect permits revealing no Basic. Preserve the Dragon payload in
    // Prizes when K1 proves the promised outlet cannot legally search:
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          "Looked at Prize cards; no selected Basic Pokemon was revealed, so Heavy Ball was discarded.");
    return true;
  }

  bool play_gladion() {
    const bool dead_prize_payload_route = prizes_known() && need_payload() &&
        !need_regi() && !need_vstar() && !need_energy() && !need_active_vstar() &&
        can_play_payload_this_turn() && !prize_payload_one_discard_outlet_has_legal_target();
    if (!dead_prize_payload_route) return play_gladion_prize_payload_original();

    // Gladion may exchange itself for a Prize card. Hide only the proven-dead Item
    // identities while the existing selector runs, then restore the exact hand when
    // it correctly declines the stranded payload route:
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    const std::vector<Card> original_hand = state_.hand;
    while (remove_one(state_.hand, Card::MysteriousTreasure)) {}
    while (remove_one(state_.hand, Card::QuickBall)) {}
    while (remove_one(state_.hand, Card::EarthenVessel)) {}
    const bool played = play_gladion_prize_payload_original();
    if (!played) state_.hand = original_hand;
    return played;
  }

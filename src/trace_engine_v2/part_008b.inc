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
    const auto present = [this, excluded](const Card card) { return card != excluded && hand_count(card) > 0; };
    return present(Card::MysteriousTreasure) || present(Card::QuickBall) || present(Card::UltraBall) ||
           present(Card::EarthenVessel) || present(Card::Serena) || present(Card::BrilliantBlender) ||
           (supporter_allowed() && present(Card::ProfessorBurnet));
  }

  Card fallback_mysterious_target_after_search_started() const {
    // Mysterious Treasure can search a Psychic or Dragon Pokémon: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dipplin TWM 127 is a Dragon Pokémon and must remain an eligible fallback: https://api.pokemontcg.io/v2/cards/sv6-127
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::Dipplin}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  Card fallback_quick_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX,
                              Card::MawileGX, Card::DialgaGX}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
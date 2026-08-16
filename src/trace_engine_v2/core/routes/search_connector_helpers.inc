  bool need_regi() const { return !has_any_regi(); }

  // The VSTAR card axis is complete once the card is in hand; evolve_best_regi
  // resolves the later game-state transition when its timing condition is legal.
  bool need_vstar() const {
    return has_regi_v() && !has_vstar() &&
           hand_count(Card::RegidragoVstar) == 0;
  }

  bool has_other_payload_outlet(const Card excluded) const {
    // A search connector can only fetch a future payload when a payload may still
    // be in deck. K1 derives this from legal deck or Prize inspection.
    if (!payload_might_be_in_deck()) return false;
    // The fetched payload is in hand, so the continuation must discard from hand.
    // Mysterious Treasure, Quick Ball, Ultra Ball, and Earthen Vessel have hand-discard
    // costs: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/sv4-163
    // Serena can discard from hand only while the turn's Supporter play remains available:
    // https://api.pokemontcg.io/v2/cards/swsh12-164 https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
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
    // Crobat V is Darkness and is excluded from this typed fallback:
    // https://api.pokemontcg.io/v2/cards/swsh3-104
    // https://github.com/FlareZ123/pokemon-sims/issues/1253
    // https://github.com/FlareZ123/pokemon-sims/issues/1949
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar,
                              Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite,
                              Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::Appletun, Card::Dipplin}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  Card fallback_quick_ball_target_after_search_started() const {
    // Quick Ball searches Basic Pokémon, and this K1 fallback preserves the
    // established modeled target order after legal deck inspection:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // Confirmed Pineco fallback coverage: https://github.com/FlareZ123/pokemon-sims/issues/1958
    for (const Card target : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX,
                              Card::CrobatV, Card::MawileGX, Card::DialgaGX,
                              Card::Pineco}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

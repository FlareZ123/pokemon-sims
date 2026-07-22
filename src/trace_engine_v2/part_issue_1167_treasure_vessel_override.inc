  std::optional<Card> strict_dci_redundant_payload_pair() const {
    // Appletun follows the same dynamic DCI rule as every other modeled
    // payload when two copies are redundant and a distinct payload remains:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1314
    for (const Card repeated : {Card::MegaDragonite, Card::Dragapult,
                                Card::GoodraVstar, Card::DialgaGX,
                                Card::Appletun}) {
      if (hand_count(repeated) < 2) continue;
      const bool distinct_payload_remains = std::any_of(
          state_.hand.begin(), state_.hand.end(), [repeated](const Card card) {
            return is_payload(card) && card != repeated;
          });
      if (distinct_payload_remains) return repeated;
    }
    return std::nullopt;
  }

  bool strict_dci_treasure_vessel_chain_live() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || !scenario_.going_first ||
        state_.turn != 2 || scenario_.max_turn < 5 ||
        !state_.supporter_used || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::TapuLeleGX ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        state_.active->tool != Tool::None || bench_space() <= 0 ||
        !prizes_known() ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::ProfessorTuro) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::Gladion) > 0 || hand_count(Card::RegidragoVstar) > 0 ||
        !need_regi() || has_vstar() || !need_energy() || !need_payload() ||
        deck_count_after_search_started(Card::RegidragoV) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) < 2 ||
        deck_count_after_search_started(Card::Gladion) == 0 ||
        prize_count_after_reveal(Card::MysteriousTreasure) == 0 ||
        !strict_dci_redundant_payload_pair()) {
      return false;
    }

    // Ordinary lower-DCI cards stay preferred. This exception is available only
    // when the normal Treasure cost selector has no legal card to spend:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1167
    return !choose_discard(false, true, true, Card::MysteriousTreasure).has_value();
  }

  bool play_strict_dci_treasure_vessel_chain() {
    if (!strict_dci_treasure_vessel_chain_live()) return false;
    const Card redundant_payload = *strict_dci_redundant_payload_pair();

    // Mysterious Treasure discards one other card, searches a Dragon Pokémon, and
    // shuffles. Two copies of the same payload are dynamically discardable because
    // a distinct modeled payload remains protected for the later strict-JIT turn:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh11-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/1167
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(redundant_payload, "Mysterious Treasure cost", "R-MT-01; P-DCI-01");
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::RegidragoV)) {
      throw std::logic_error("projected Mysterious Treasure Regidrago V target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          "Discarded one redundant Dragon payload and searched Regidrago V.");

    // A Basic Pokémon may be played onto an open Bench before the next Item. This
    // establishes the Energy target and preserves Active Tapu Lele-GX for Turo:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1167
    if (!bench_from_hand(Card::RegidragoV, false)) {
      throw std::logic_error("projected Regidrago V Bench play failed");
    }

    // Earthen Vessel discards the second redundant copy and may search two Basic
    // Energy of the same type. Taking two Grass preserves the held Fire for the T4
    // manual attachment after the two Grass attachments on T2 and T3:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1167
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(redundant_payload, "Earthen Vessel cost", "R-EV-01; P-DCI-01");
    record_deck_search_knowledge("Earthen Vessel");
    for (int copy = 0; copy < 2; ++copy) {
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("projected Earthen Vessel Grass target disappeared");
      }
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-DCI-01",
          "Discarded the second redundant Dragon payload and searched two Grass Energy.");
    return true;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    // The source-bounded paired paid-search route is evaluated before the ordinary
    // strict-DCI selector, which correctly protects every payload in isolation:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://github.com/FlareZ123/pokemon-sims/issues/1167
    if (play_strict_dci_treasure_vessel_chain()) return true;
    return play_mysterious_treasure_issue1167_original(permit_payload);
  }

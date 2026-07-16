  bool turbo_is_item(const Card card) const {
    switch (card) {
      case Card::BrilliantBlender:
      case Card::MysteriousTreasure:
      case Card::QuickBall:
      case Card::UltraBall:
      case Card::EvolutionIncense:
      case Card::EarthenVessel:
      case Card::FieldBlower:
      case Card::HisuianHeavyBall:
      case Card::BattleVipPass:
        return true;
      default:
        return false;
    }
  }

  bool play_battle_vip_pass() {
    if (item_locked() || state_.turn != 1 || hand_count(Card::BattleVipPass) == 0 ||
        bench_space() <= 0) {
      return false;
    }

    std::vector<Card> targets;
    int projected_regi = in_play_count(Card::RegidragoV);
    while (bench_space() > static_cast<int>(targets.size()) && projected_regi < 2 &&
           count_of(state_.deck, Card::RegidragoV) >
               static_cast<int>(std::count(targets.begin(), targets.end(), Card::RegidragoV))) {
      targets.push_back(Card::RegidragoV);
      ++projected_regi;
    }
    if (targets.empty() && bench_space() > 0 && !in_play(Card::LatiasEx) &&
        count_of(state_.deck, Card::LatiasEx) > 0 && state_.active &&
        state_.active->card != Card::RegidragoV) {
      targets.push_back(Card::LatiasEx);
    }
    if (targets.empty()) return false;

    remove_one(state_.hand, Card::BattleVipPass);
    state_.discard.push_back(Card::BattleVipPass);
    record_deck_search_knowledge("Battle VIP Pass");
    for (const Card target : targets) {
      if (!remove_one(state_.deck, target)) continue;
      state_.bench.push_back(Pokemon{target, state_.turn});
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-GAME-ITEM",
          "Battle VIP Pass put Basic Pokémon onto the Bench: " + join_cards(targets) +
          ". Card text: https://api.pokemontcg.io/v2/cards/swsh8-225");
    return true;
  }

  bool play_pokestop() {
    if (hand_count(Card::PokeStop) == 0 || state_.stadium == Stadium::PokeStop) return false;
    remove_one(state_.hand, Card::PokeStop);
    state_.stadium = Stadium::PokeStop;
    state_.path_lock_removed = true;
    state_.stadium_ability_used = false;
    trace("PLAY STADIUM", "R-GAME-STADIUM",
          "Played PokéStop. Card text: https://api.pokemontcg.io/v2/cards/pgo-68");
    return true;
  }

  bool use_pokestop() {
    if (state_.stadium != Stadium::PokeStop || state_.stadium_ability_used ||
        state_.deck.empty()) {
      return false;
    }
    state_.stadium_ability_used = true;
    std::vector<Card> items;
    std::vector<Card> discarded;
    for (int index = 0; index < 3 && !state_.deck.empty(); ++index) {
      const Card card = state_.deck.back();
      state_.deck.pop_back();
      if (turbo_is_item(card)) {
        state_.hand.push_back(card);
        items.push_back(card);
      } else {
        state_.discard.push_back(card);
        state_.discarded_this_turn.push_back(card);
        discarded.push_back(card);
      }
    }
    trace("USE STADIUM", "R-GAME-STADIUM",
          "PokéStop put Items into hand: " + join_cards(items) +
          "; discarded: " + join_cards(discarded) +
          ". Card text: https://api.pokemontcg.io/v2/cards/pgo-68");
    return true;
  }

  bool play_carmine() {
    if (state_.turn != 1 || !scenario_.going_first || state_.supporter_used ||
        hand_count(Card::Carmine) == 0 || state_.turn_ended) {
      return false;
    }
    const int missing_axes = (need_regi() ? 1 : 0) + (need_vstar() ? 1 : 0) +
                             (need_energy() ? 1 : 0) + (need_payload() ? 1 : 0);
    if (missing_axes < 2) return false;
    if (strict_payload_timing() && std::any_of(state_.hand.begin(), state_.hand.end(),
                                               is_payload)) {
      return false;
    }

    remove_one(state_.hand, Card::Carmine);
    state_.discard.push_back(Card::Carmine);
    state_.discard.insert(state_.discard.end(), state_.hand.begin(), state_.hand.end());
    state_.discarded_this_turn.insert(state_.discarded_this_turn.end(),
                                      state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    for (int draw = 0; draw < 5; ++draw) draw_card("Carmine");
    state_.supporter_used = true;
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Used Carmine on the first turn while going first. Card text: "
          "https://api.pokemontcg.io/v2/cards/sv8pt5-103");
    return true;
  }

  void play_turbo_external_actions() {
    play_pokestop();
    use_pokestop();
    play_battle_vip_pass();
  }

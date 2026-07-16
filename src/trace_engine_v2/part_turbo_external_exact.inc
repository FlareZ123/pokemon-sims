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
      case Card::PokemonCommunication:
      case Card::Pokegear:
      case Card::NestBall:
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

  bool turbo_is_trainer(const Card card) const {
    return turbo_is_item(card) || is_supporter(card) || card == Card::ForestSealStone ||
           card == Card::Powerglass || card == Card::PokeStop ||
           card == Card::ChaoticSwell || card == Card::PathToPeak;
  }

  int turbo_trainer_priority(const Card card) const {
    if (need_payload() && (card == Card::BrilliantBlender || card == Card::ProfessorBurnet)) return 100;
    if (need_energy() && (card == Card::Crispin || card == Card::EarthenVessel)) return 95;
    if (need_vstar() && (card == Card::EvolutionIncense || card == Card::MysteriousTreasure ||
                         card == Card::UltraBall || card == Card::Arven)) return 90;
    if (need_regi() && (card == Card::QuickBall || card == Card::NestBall ||
                        card == Card::UltraBall)) return 85;
    if (need_active_vstar() && card == Card::TateLiza) return 80;
    if (card == Card::ProfessorsResearch) return 65;
    if (card == Card::Lillie) return 60;
    if (card == Card::Pokegear) return 55;
    if (card == Card::ForestSealStone) return 45;
    return 10;
  }

  Card best_supporter_from_revealed(const std::vector<Card>& cards) const {
    Card best = Card::Carmine;
    int best_priority = -1;
    for (const Card card : cards) {
      if (!is_supporter(card)) continue;
      const int priority = turbo_trainer_priority(card);
      if (priority > best_priority) {
        best = card;
        best_priority = priority;
      }
    }
    return best_priority >= 0 ? best : Card::TurboBlank;
  }

  bool play_pokegear() {
    if (item_locked() || hand_count(Card::Pokegear) == 0 || state_.deck.empty()) return false;
    remove_one(state_.hand, Card::Pokegear);
    state_.discard.push_back(Card::Pokegear);
    std::vector<Card> revealed;
    for (int i = 0; i < 7 && !state_.deck.empty(); ++i) {
      revealed.push_back(state_.deck.back());
      state_.deck.pop_back();
    }
    const Card chosen = best_supporter_from_revealed(revealed);
    if (chosen != Card::TurboBlank) {
      const auto it = std::find(revealed.begin(), revealed.end(), chosen);
      state_.hand.push_back(chosen);
      revealed.erase(it);
    }
    state_.deck.insert(state_.deck.end(), revealed.begin(), revealed.end());
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-GAME-ITEM",
          chosen != Card::TurboBlank ? "Pokégear 3.0 found " + std::string(name(chosen)) + ". Card text: https://api.pokemontcg.io/v2/cards/sm10-182"
                                     : "Pokégear 3.0 found no Supporter. Card text: https://api.pokemontcg.io/v2/cards/sm10-182");
    return true;
  }

  bool play_nest_ball() {
    if (item_locked() || hand_count(Card::NestBall) == 0 || bench_space() <= 0) return false;
    Card target = Card::TurboBlank;
    if (need_regi() && count_of(state_.deck, Card::RegidragoV) > 0) target = Card::RegidragoV;
    else if (need_active_vstar() && !in_play(Card::LatiasEx) &&
             ability_available_for_pokemon(Card::LatiasEx) && count_of(state_.deck, Card::LatiasEx) > 0) target = Card::LatiasEx;
    else if (need_energy() && !in_play(Card::Oricorio) && count_of(state_.deck, Card::Oricorio) > 0) target = Card::Oricorio;
    if (target == Card::TurboBlank) return false;
    remove_one(state_.hand, Card::NestBall);
    state_.discard.push_back(Card::NestBall);
    record_deck_search_knowledge("Nest Ball");
    if (!remove_one(state_.deck, target)) throw std::logic_error("Nest Ball target disappeared");
    state_.bench.push_back(Pokemon{target, state_.turn});
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-GAME-ITEM", "Nest Ball Benched " + std::string(name(target)) + ". Card text: https://api.pokemontcg.io/v2/cards/sv1-181");
    return true;
  }

  std::optional<Card> communication_cost() const {
    for (const Card card : {Card::Dipplin, Card::MawileGX, Card::Oricorio,
                            Card::TapuLeleGX, Card::LatiasEx, Card::RegidragoV}) {
      if (hand_count(card) > 0 && !(card == Card::RegidragoV && need_regi()) &&
          !(card == Card::LatiasEx && need_active_vstar()) &&
          !(card == Card::Oricorio && need_energy())) return card;
    }
    if (hand_count(Card::RegidragoVstar) > 1) return Card::RegidragoVstar;
    return std::nullopt;
  }

  bool play_pokemon_communication() {
    if (item_locked() || hand_count(Card::PokemonCommunication) == 0) return false;
    const auto cost = communication_cost();
    if (!cost) return false;
    Card target = Card::TurboBlank;
    if (need_regi() && count_of(state_.deck, Card::RegidragoV) > 0) target = Card::RegidragoV;
    else if (need_vstar() && count_of(state_.deck, Card::RegidragoVstar) > 0) target = Card::RegidragoVstar;
    else if (need_active_vstar() && count_of(state_.deck, Card::LatiasEx) > 0) target = Card::LatiasEx;
    else if (need_energy() && count_of(state_.deck, Card::Oricorio) > 0) target = Card::Oricorio;
    else if (needs_tapu_connector() && count_of(state_.deck, Card::TapuLeleGX) > 0) target = Card::TapuLeleGX;
    if (target == Card::TurboBlank) return false;
    remove_one(state_.hand, Card::PokemonCommunication);
    state_.discard.push_back(Card::PokemonCommunication);
    remove_one(state_.hand, *cost);
    state_.deck.push_back(*cost);
    record_deck_search_knowledge("Pokémon Communication");
    if (!move_deck_to_hand(target)) throw std::logic_error("Pokémon Communication target disappeared");
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-GAME-ITEM", "Pokémon Communication shuffled " + std::string(name(*cost)) + " and found " + std::string(name(target)) + ". Card text: https://api.pokemontcg.io/v2/cards/sm9-152");
    return true;
  }

  bool play_adventurers_discovery() {
    if (!supporter_allowed() || hand_count(Card::AdventurersDiscovery) == 0 ||
        (!need_regi() && !need_vstar())) return false;
    remove_one(state_.hand, Card::AdventurersDiscovery);
    state_.discard.push_back(Card::AdventurersDiscovery);
    state_.supporter_used = true;
    record_deck_search_knowledge("Adventurer's Discovery");
    std::vector<Card> targets;
    while (targets.size() < 3U && need_regi() && count_of(state_.deck, Card::RegidragoV) > 0) {
      if (!move_deck_to_hand(Card::RegidragoV)) break;
      targets.push_back(Card::RegidragoV);
      break;
    }
    while (targets.size() < 3U && count_of(state_.deck, Card::RegidragoVstar) > 0) {
      if (!move_deck_to_hand(Card::RegidragoVstar)) break;
      targets.push_back(Card::RegidragoVstar);
      if (!need_vstar()) break;
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER", "Adventurer's Discovery found: " + join_cards(targets) + ". Card text: https://api.pokemontcg.io/v2/cards/swsh8-224");
    return true;
  }

  bool play_professors_research() {
    if (!supporter_allowed() || hand_count(Card::ProfessorsResearch) == 0) return false;
    const int missing_axes = (need_regi() ? 1 : 0) + (need_vstar() ? 1 : 0) +
                             (need_energy() ? 1 : 0) + (need_payload() ? 1 : 0) +
                             (need_active_vstar() ? 1 : 0);
    const bool payload_discard = can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (missing_axes < 2 && !payload_discard && state_.hand.size() > 3U) return false;
    remove_one(state_.hand, Card::ProfessorsResearch);
    state_.discard.push_back(Card::ProfessorsResearch);
    state_.discard.insert(state_.discard.end(), state_.hand.begin(), state_.hand.end());
    state_.discarded_this_turn.insert(state_.discarded_this_turn.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    for (int i = 0; i < 7; ++i) draw_card("Professor's Research");
    state_.supporter_used = true;
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER", "Professor's Research discarded the hand and drew 7. Card text: https://api.pokemontcg.io/v2/cards/swsh1-178");
    return true;
  }

  bool play_lillie() {
    if (!supporter_allowed() || hand_count(Card::Lillie) == 0 || !setup_axis_missing()) return false;
    const std::size_t target = state_.turn == 1 ? 8U : 6U;
    if (state_.hand.size() >= target) return false;
    remove_one(state_.hand, Card::Lillie);
    state_.discard.push_back(Card::Lillie);
    state_.supporter_used = true;
    while (state_.hand.size() < target && !state_.deck.empty()) draw_card("Lillie");
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER", "Lillie drew to " + std::to_string(target) + ". Card text: https://api.pokemontcg.io/v2/cards/sm1-122");
    return true;
  }

  bool play_turbo_external_supporter() {
    if (play_adventurers_discovery()) return true;
    if (play_professors_research()) return true;
    return play_lillie();
  }

  void play_turbo_external_actions() {
    play_pokestop();
    use_pokestop();
    play_pokegear();
    play_battle_vip_pass();
    play_nest_ball();
    play_pokemon_communication();
  }

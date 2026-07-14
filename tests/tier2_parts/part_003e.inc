void ultra_ball_holds_for_live_crispin() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::RegidragoVstar, Card::UltraBall, Card::Crispin, Card::Dipplin, Card::Grass, Card::Grass};
  s.deck = {Card::Oricorio, Card::Grass, Card::Fire};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("ultra-ball-hold-for-crispin"), s);
  expect(!EngineTestAccess::play_ultra_ball(f.engine, false), "Ultra Ball should not discard two cards for Oricorio when Crispin is a direct Energy line.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::UltraBall), "Ultra Ball should remain in hand.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(EngineTestAccess::state(f.engine).active->fire == 1, "Crispin should attach the missing Fire directly.");
}

void costed_search_accepts_distinct_ultra_second_cost() {
  State s = st(2, Card::RegidragoVstar, 1, 2, 1);
  s.hand = {Card::MysteriousTreasure, Card::UltraBall, Card::UltraBall, Card::UltraBall};
  s.deck = {Card::MegaDragonite, Card::TapuLeleGX};
  Fixture f(sc("costed-search-distinct-ultra-second-cost"), s);

  // Mysterious Treasure may discard one Ultra Ball and search a Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, true),
         "Mysterious Treasure should recognize the surviving two-Ultra-Ball payload continuation.");
  State& after_search = EngineTestAccess::state(f.engine);
  expect(has(after_search.hand, Card::MegaDragonite), "The Dragon payload should enter hand.");
  expect(std::count(after_search.hand.begin(), after_search.hand.end(), Card::UltraBall) == 2,
         "Two distinct Ultra Ball copies should survive the first search.");

  // One Ultra Ball may discard the fetched payload and the other Ultra Ball as its
  // two other cards: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  expect(EngineTestAccess::play_ultra_ball(f.engine, true),
         "The surviving Ultra Ball should use the payload and distinct Ultra Ball as costs.");
  expect(has(EngineTestAccess::state(f.engine).discard, Card::MegaDragonite),
         "The fetched Dragon must remain in the current-turn discard for Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136");
}


void test_full_bench_blocks_basic_search_connectors() {
  State s = state(2, Card::RegidragoV, 1, 2, 1);
  s.bench = {
      Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None},
      Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None},
      Pokemon{Card::Oricorio, 1, 0, 0, Tool::None},
      Pokemon{Card::MawileGX, 1, 0, 0, Tool::None},
      Pokemon{Card::DialgaGX, 1, 0, 0, Tool::None},
  };
  s.hand = {Card::QuickBall, Card::MysteriousTreasure, Card::Dipplin};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("full-bench-no-latias-search"), s);

  expect(!EngineTestAccess::play_quick_ball(f.engine, false),
         "Quick Ball must not be spent to fetch Latias when the Bench is full.");
  expect(!EngineTestAccess::play_mysterious_treasure(f.engine, false),
         "Mysterious Treasure must not be spent to fetch Latias when the Bench is full.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::QuickBall) && contains(after.hand, Card::MysteriousTreasure),
         "Full-Bench Basic search connectors should remain in hand for a later legal board state.");
}

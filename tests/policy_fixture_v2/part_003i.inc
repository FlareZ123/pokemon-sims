
void test_steven_resolve_reserves_gladion_for_prized_vstar() {
  State s = state(1, Card::RegidragoV, 0);
  s.hand = {Card::StevensResolve};
  // Crispin's live Energy route needs both Basic Energy types in the deck: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender can discard this deck's Mega Dragonite payload: https://api.pokemontcg.io/v2/cards/sv8-164
  s.deck = {Card::Gladion, Card::Crispin, Card::BrilliantBlender, Card::MegaDragonite, Card::Grass, Card::Fire};
  s.prizes = {Card::RegidragoVstar};
  Fixture f(scenario("steven-prized-vstar-k1"), s);

  expect(EngineTestAccess::play_steven(f.engine),
         "Steven's Resolve should be playable going second on turn one with three setup axes missing.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::Gladion) && contains(after.hand, Card::Crispin) &&
         contains(after.hand, Card::BrilliantBlender),
         "After legal deck inspection, Steven should take Gladion for prized VSTAR plus the Energy and payload outlets.");
  expect(!contains(after.hand, Card::RegidragoVstar),
         "The prized VSTAR should be recognized as absent from deck rather than searched unsuccessfully.");
  expect(after.turn_ended, "Steven's Resolve must still end the turn after the K1 selection.");
}

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

void test_strict_serena_uses_three_safe_discards() {
  State s = state(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::Serena, Card::Dipplin, Card::Grass, Card::Grass, Card::Fire, Card::Fire};
  s.deck = {Card::RegidragoVstar, Card::MawileGX, Card::Guzma};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("strict-serena-three-discards"), s);

  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::Serena) && contains(after.discard, Card::Dipplin) &&
         contains(after.discard, Card::Grass) && contains(after.discard, Card::Fire),
         "Strict Serena should discard Dipplin plus two surplus Energy cards when all are policy-safe.");
  expect(after.hand.size() == 5U && contains(after.hand, Card::RegidragoVstar),
         "After three legal discards, Serena should draw back to five and find the compact-deck VSTAR.");
}

void test_heavy_ball_oricorio_preserves_item_lock_burnet() {
  State s = state(2, Card::RegidragoVstar, 1, 2, 0);
  s.hand = {Card::HisuianHeavyBall, Card::ProfessorBurnet};
  s.deck = {Card::Fire, Card::MegaDragonite, Card::Crispin, Card::Grass};
  s.prizes = {Card::TapuLeleGX, Card::Oricorio};
  Fixture f(scenario("heavy-ball-oricorio-over-tapu", DciProfile::StrictJit, LockMode::FullItem), s);

  expect(EngineTestAccess::play_heavy_ball(f.engine),
         "Heavy Ball should resolve and inspect the two live prized Basic connectors.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Oricorio),
         "With Burnet already in hand under Item lock, Heavy Ball should recover Oricorio over Tapu Lele-GX.");
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::Oricorio, true),
         "The recovered Oricorio should Bench and use Vital Dance.");
  expect(EngineTestAccess::attach_manual(f.engine),
         "The searched Fire should be manually attached to finish GGF.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::ProfessorBurnet) && contains(after.discard, Card::MegaDragonite),
         "Preserved Burnet should discard the Dragon payload in the same turn.");
  expect(after.active->fire == 1,
         "The Oricorio/manual line should complete the final Fire attachment.");
}

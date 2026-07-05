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

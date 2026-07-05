void fss_holds_for_live_crispin() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::RegidragoVstar, Card::Crispin};
  s.deck = {Card::Grass, Card::Fire, Card::Oricorio};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("fss-hold-for-crispin"), s);
  expect(!EngineTestAccess::use_fss(f.engine), "FSS should be preserved when Crispin is the sole remaining direct Energy line.");
  expect(!EngineTestAccess::state(f.engine).vstar_power_used, "The VSTAR Power should remain unused for later turns.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(EngineTestAccess::state(f.engine).active->fire == 1, "Crispin should attach the missing Fire directly.");
}

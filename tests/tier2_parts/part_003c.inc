void tapu_avoids_redundant_arven() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TapuLeleGX, Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tapu-vstar-in-hand"), s);
  expect(!EngineTestAccess::bench_tapu_if_useful(f.engine), "Tapu should remain unused.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::TapuLeleGX), "Tapu should remain in hand.");
}

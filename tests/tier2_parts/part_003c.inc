void tapu_avoids_redundant_arven() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TapuLeleGX, Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tapu-vstar-in-hand"), s);
  expect(!EngineTestAccess::bench_tapu_if_useful(f.engine), "Tapu should remain unused.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::TapuLeleGX), "Tapu should remain in hand.");
}

void tapu_uses_gladion_for_known_prized_vstar() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TapuLeleGX, Card::Arven, Card::Crispin};
  s.deck = {Card::Gladion, Card::Arven, Card::Crispin};
  s.prizes = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tapu-known-prized-vstar"), s);
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true), "Tapu should enter the Bench and resolve Wonder Tag.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Gladion), "Wonder Tag should select Gladion after deck inspection.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::RegidragoVstar), "Gladion should recover the known prized VSTAR.");
  expect(has(a.hand, Card::Arven) && has(a.hand, Card::Crispin), "The direct Gladion route should preserve slower Supporters.");
}

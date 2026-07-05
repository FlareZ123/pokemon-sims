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

void crispin_beats_known_prized_oricorio() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::MysteriousTreasure, Card::MegaDragonite, Card::Gladion, Card::Crispin, Card::RegidragoVstar};
  s.deck = {Card::TapuLeleGX, Card::Grass, Card::Fire};
  s.prizes = {Card::Oricorio};
  Fixture f(sc("crispin-over-prized-oricorio"), s);
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, true), "Mysterious Treasure should establish K1 legally.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::Crispin), "Crispin should beat a known prized Oricorio connector.");
  expect(has(a.hand, Card::Gladion), "Gladion should remain available after the direct Energy line.");
  expect(a.active->fire == 1, "Crispin should attach the missing Fire immediately.");
}

void fss_holds_for_tate_switch() {
  State s = st(2, Card::RegidragoV, 1);
  s.active->tool = Tool::ForestSealStone;
  s.bench.push_back(Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None});
  s.hand = {Card::TateLiza};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("fss-hold-for-tate"), s);
  expect(!EngineTestAccess::use_fss(f.engine), "FSS should be preserved when Tate and Liza alone fixes Active positioning.");
  expect(!EngineTestAccess::state(f.engine).vstar_power_used, "The VSTAR Power should remain unused.");
}

void fss_uses_gladion_for_known_prized_vstar() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::Gladion, Card::Arven, Card::Crispin};
  s.prizes = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("fss-known-prized-vstar"), s);
  expect(EngineTestAccess::use_fss(f.engine), "FSS should resolve as a legal deck inspection.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Gladion), "FSS should select Gladion for the known prized VSTAR.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(has(EngineTestAccess::state(f.engine).hand, Card::RegidragoVstar), "Gladion should recover the prized VSTAR after FSS inspection.");
}

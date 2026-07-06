void fss_uses_oricorio_compression() {
  State s = st(2, Card::RegidragoV, 1, 0, 0);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::Oricorio, Card::Grass, Card::Fire};
  Fixture f(sc("tier2-fss-oricorio"), s);
  expect(EngineTestAccess::use_fss(f.engine), "FSS should resolve.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Oricorio), "FSS should take Oricorio instead of one Energy.");
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::Oricorio, true), "Oricorio should enter Bench.");
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::Grass) && has(a.hand, Card::Fire), "Vital Dance should access both Energy.");
}

void fss_uses_vessel_over_arven() {
  State s = st(2, Card::RegidragoV, 1, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::Dipplin};
  s.deck = {Card::EarthenVessel, Card::Arven, Card::Grass, Card::Fire};
  Fixture f(sc("tier2-fss-vessel"), s);
  expect(EngineTestAccess::use_fss(f.engine), "FSS should resolve.");
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::EarthenVessel) && !has(a.hand, Card::Arven), "FSS should choose direct Vessel over Arven.");
  expect(EngineTestAccess::play_earthen_vessel(f.engine, false), "Vessel should be immediately playable.");
}

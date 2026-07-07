
void crispin_beats_arven() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::Crispin, Card::Arven, Card::RegidragoVstar};
  s.deck = {Card::Grass, Card::Fire, Card::EarthenVessel, Card::ForestSealStone};
  Fixture f(sc("tier2-crispin-over-arven"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::Crispin), "Crispin should be selected before Arven.");
  expect(has(a.hand, Card::Arven), "Arven should be preserved.");
  expect(a.active->grass == 2 && a.active->fire == 1, "Crispin should complete GGF now.");
  expect(EngineTestAccess::evolve_best_regi(f.engine), "The powered prior-turn V should evolve now.");
}

void steven_beats_arven() {
  State s = st(1, Card::RegidragoV, 0);
  s.hand = {Card::StevensResolve, Card::Arven};
  // The Steven route uses Crispin only with a searchable Grass/Fire pair: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender can discard this deck's Mega Dragonite payload: https://api.pokemontcg.io/v2/cards/sv8-164
  s.deck = {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender, Card::MegaDragonite, Card::ForestSealStone, Card::Grass, Card::Fire};
  Fixture f(sc("tier2-steven-over-arven", DciProfile::StrictJit, LockMode::None, false), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::StevensResolve), "Steven should be selected before Arven.");
  expect(has(a.hand, Card::RegidragoVstar) && has(a.hand, Card::Crispin) && has(a.hand, Card::BrilliantBlender), "Steven should cover VSTAR, Energy, and payload.");
  expect(has(a.hand, Card::Arven) && a.turn_ended, "Arven should remain and Steven should end the turn.");
}

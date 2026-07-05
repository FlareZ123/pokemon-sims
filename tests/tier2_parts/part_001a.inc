void search_then_known_gladion_beats_supporters() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::Gladion, Card::StevensResolve, Card::Arven};
  s.deck = {Card::TapuLeleGX}; s.prizes = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite}; s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tier2-seek-known-gladion"), s);
  expect(!EngineTestAccess::prizes_known(f.engine), "Prizes must begin unknown.");
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, false), "Treasure should search legally.");
  expect(EngineTestAccess::prizes_known(f.engine), "Search should establish K1 knowledge.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::RegidragoVstar), "Known prized VSTAR should be taken by Gladion.");
  expect(has(a.hand, Card::StevensResolve) && has(a.hand, Card::Arven), "Gladion should preserve contested Supporters.");
}

void k0_does_not_peek_prizes() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::RegidragoVstar, Card::Crispin, Card::Gladion, Card::Arven};
  s.deck = {Card::Grass, Card::Fire, Card::EarthenVessel}; s.prizes = {Card::BrilliantBlender};
  Fixture f(sc("tier2-k0-no-oracle"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::Crispin) && has(a.hand, Card::Gladion), "Unseen prize information must not displace the live Crispin line.");
}

void blind_gladion_is_critical_only() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::Gladion}; s.prizes = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::MawileGX, Card::Guzma};
  s.discard = {Card::MegaDragonite}; s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tier2-blind-gladion"), s);
  EngineTestAccess::choose_supporter(f.engine);
  expect(has(EngineTestAccess::state(f.engine).hand, Card::RegidragoVstar), "Critical blind Gladion should reveal and recover the actual VSTAR route.");
  expect(EngineTestAccess::prizes_known(f.engine), "Gladion reveal should establish K1 knowledge.");
}

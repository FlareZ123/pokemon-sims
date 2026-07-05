void fss_uses_direct_arven_not_tapu_chain() {
  State s = st(2, Card::RegidragoV, 1, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::RegidragoVstar};
  s.deck = {Card::Arven, Card::MysteriousTreasure, Card::TapuLeleGX};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tier2-fss-direct-arven"), s);
  expect(EngineTestAccess::use_fss(f.engine), "FSS should resolve.");
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::Arven) && !has(a.hand, Card::MysteriousTreasure) && !has(a.hand, Card::TapuLeleGX), "FSS should avoid the slower Treasure -> Tapu -> Arven chain.");
}

void fss_uses_blender_for_payload() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::RegidragoVstar};
  s.deck = {Card::BrilliantBlender, Card::Arven, Card::MegaDragonite};
  Fixture f(sc("tier2-fss-blender"), s);
  expect(EngineTestAccess::use_fss(f.engine), "FSS should resolve.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::BrilliantBlender), "FSS should take Blender directly.");
  expect(EngineTestAccess::play_brilliant_blender(f.engine), "Blender should play now.");
  expect(has(EngineTestAccess::state(f.engine).discard, Card::MegaDragonite), "Blender should discard payload now.");
}

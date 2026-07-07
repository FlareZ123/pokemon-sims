
void test_wonder_tag_skips_known_dead_crispin() {
  State s = state(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::TapuLeleGX};
  s.deck = {Card::Crispin, Card::Serena};
  s.prizes = {Card::Fire};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("wonder-tag-dead-crispin"), s);

  expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true),
         "Wonder Tag should legally inspect the deck after Tapu Lele-GX is Benched.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::Serena) && !contains(after.hand, Card::Crispin),
         "Wonder Tag must not select Crispin after K1 proves no needed Fire remains in deck.");
}

void test_steven_skips_known_dead_crispin() {
  State s = state(1, Card::RegidragoV, 0);
  s.hand = {Card::StevensResolve};
  // Brilliant Blender can discard this deck's Mega Dragonite payload after Steven's Resolve: https://api.pokemontcg.io/v2/cards/sv8-164
  s.deck = {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender, Card::MysteriousTreasure, Card::MegaDragonite};
  s.prizes = {Card::Grass, Card::Fire};
  Fixture f(scenario("steven-dead-crispin"), s);

  expect(EngineTestAccess::play_steven(f.engine),
         "Steven's Resolve should be playable when VSTAR and payload axes are missing.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(!contains(after.hand, Card::Crispin),
         "Steven must not select Crispin after its deck inspection proves both needed Energy types are prized.");
  expect(contains(after.hand, Card::RegidragoVstar) && contains(after.hand, Card::BrilliantBlender),
         "Steven should retain legal live setup cards instead of the dead Energy Supporter.");
}

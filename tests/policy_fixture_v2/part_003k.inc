
void test_legacy_star_recovered_blender_is_played_same_turn() {
  State s = state(2, Card::RegidragoVstar, 1, 2, 1);
  // Engine pops from the back. Leave Mega Dragonite ex beneath the seven cards
  // Legacy Star discards so recovered Brilliant Blender has a live deck target.
  s.deck = {Card::MegaDragonite, Card::Grass, Card::Fire, Card::Dipplin,
            Card::MawileGX, Card::Guzma, Card::Channeler, Card::BrilliantBlender};
  Fixture f(scenario("legacy-recovered-blender"), s);

  EngineTestAccess::run_turn(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(EngineTestAccess::outcome(f.engine).used_legacy_star,
         "Legacy Star should be the only available payload line.");
  expect(contains(after.discard, Card::MegaDragonite),
         "Recovered Brilliant Blender must be playable later in the same turn and discard Mega Dragonite ex.");
  expect(EngineTestAccess::payload_ready(f.engine),
         "The Blender discard must satisfy strict current-turn payload timing.");
}

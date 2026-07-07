
void test_legacy_star_recovered_blender_is_played_same_turn() {
  State s = state(2, Card::RegidragoVstar, 1, 2, 1);
  // Engine pops from the back. Leave Mega Dragonite ex beneath the seven cards
  // Legacy Star discards so recovered Brilliant Blender has a live deck target.
  s.deck = {Card::MegaDragonite, Card::Grass, Card::Fire, Card::Dipplin,
            Card::MawileGX, Card::Guzma, Card::Channeler, Card::BrilliantBlender};
  s.prizes = {Card::Powerglass};
  Fixture f(scenario("legacy-recovered-blender"), s);

  EngineTestAccess::run_turn(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(EngineTestAccess::outcome(f.engine).used_legacy_star,
         "Legacy Star should be the only available payload line.");
  expect(contains(after.discard, Card::MegaDragonite),
         "Recovered Brilliant Blender must be playable later in the same turn and discard Mega Dragonite ex.");
  expect(EngineTestAccess::payload_ready(f.engine),
         "The Blender discard must satisfy strict current-turn payload timing.");

  // Legacy Star discards the top 7 cards, then may recover up to 2 discarded cards:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // This swaps two hidden cards while keeping the same remaining-deck size. At K0,
  // the policy may not use that hidden placement to choose Blender recovery.
  State payload_prized = state(2, Card::RegidragoVstar, 1, 2, 1);
  payload_prized.deck = {Card::Powerglass, Card::Grass, Card::Fire, Card::Dipplin,
                         Card::MawileGX, Card::Guzma, Card::Channeler, Card::BrilliantBlender};
  payload_prized.prizes = {Card::MegaDragonite};
  Fixture hidden_prize_fixture(scenario("legacy-blender-k0-prized-payload"), payload_prized);

  EngineTestAccess::run_turn(hidden_prize_fixture.engine);
  expect(EngineTestAccess::outcome(hidden_prize_fixture.engine).used_legacy_star,
         "Legacy Star should resolve from the identical public K0 state.");
  expect(EngineTestAccess::outcome(hidden_prize_fixture.engine).used_blender,
         "K0 policy must recover and attempt the same Blender route without peeking at hidden payload placement.");
}

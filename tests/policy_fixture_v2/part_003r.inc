void test_tapu_fetches_burnet_without_item_lock_when_blender_is_not_live() {
  State s = state(2, Card::RegidragoVstar, 0, 2, 1);
  s.hand = {Card::TapuLeleGX};
  s.deck = {Card::ProfessorBurnet, Card::MegaDragonite};
  // Wonder Tag may search the deck for a Supporter: https://api.pokemontcg.io/v2/cards/sm2-60
  Fixture f(scenario("tapu-clear-state-burnet"), s);
  expect(EngineTestAccess::bench_tapu_if_useful(f.engine),
         "Tapu Lele-GX should bench when Burnet is the only immediate payload bridge.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::ProfessorBurnet),
         "Wonder Tag should fetch Professor Burnet when Blender is not a live payload outlet.");
  expect(!after.supporter_used, "Wonder Tag must preserve the Supporter play for the fetched Burnet.");
}

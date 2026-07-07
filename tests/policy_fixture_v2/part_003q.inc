void test_live_burnet_preempts_arven_when_blender_is_absent() {
  State s = state(2, Card::RegidragoVstar, 0, 2, 1);
  s.hand = {Card::Arven, Card::ProfessorBurnet};
  s.deck = {Card::MegaDragonite};
  // Professor Burnet searches and discards a payload directly: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  Fixture f(scenario("burnet-before-arven"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(after.supporter_used && contains(after.discard, Card::ProfessorBurnet),
         "A live Professor Burnet must be chosen before Arven.");
  expect(contains(after.hand, Card::Arven),
         "Arven must remain available after Burnet completes the payload axis.");
  expect(contains(after.discard, Card::MegaDragonite),
         "Professor Burnet must put the Dragon payload into discard.");
}

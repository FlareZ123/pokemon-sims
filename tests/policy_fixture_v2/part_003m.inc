
void test_lusamine_recovers_duplicate_supporters_and_stadiums() {
  State duplicates = state(2, Card::RegidragoV, 1);
  duplicates.hand = {Card::Lusamine};
  duplicates.discard = {Card::Crispin, Card::Crispin, Card::PathToPeak};
  Fixture duplicate_fixture(scenario("lusamine-duplicate-supporter"), duplicates);
  expect(EngineTestAccess::play_lusamine(duplicate_fixture.engine),
         "Lusamine should be playable with eligible cards in discard.");
  const State& after_duplicates = EngineTestAccess::state(duplicate_fixture.engine);
  expect(count(after_duplicates.hand, Card::Crispin) == 2 && !contains(after_duplicates.hand, Card::PathToPeak),
         "Lusamine may recover two copies of the same Supporter before lower-priority cards.");

  State stadiums = state(2, Card::RegidragoV, 1);
  stadiums.hand = {Card::Lusamine};
  stadiums.discard = {Card::PathToPeak, Card::ChaoticSwell};
  Fixture stadium_fixture(scenario("lusamine-stadiums"), stadiums);
  expect(EngineTestAccess::play_lusamine(stadium_fixture.engine),
         "Lusamine should recover eligible Stadium cards.");
  const State& after_stadiums = EngineTestAccess::state(stadium_fixture.engine);
  expect(contains(after_stadiums.hand, Card::PathToPeak) && contains(after_stadiums.hand, Card::ChaoticSwell),
         "Lusamine should recover both Stadium cards under its any-combination text.");
}


void test_lusamine_recovers_only_live_axis_connectors() {
  State duplicates = state(2, Card::RegidragoV, 1);
  duplicates.hand = {Card::Lusamine};
  duplicates.discard = {Card::Crispin, Card::Crispin, Card::PathToPeak};
  // Lusamine puts 2 Supporter and/or Stadium cards into hand once its live
  // Crispin recovery makes the Supporter play worthwhile: https://api.pokemontcg.io/v2/cards/sm4-96
  Fixture duplicate_fixture(scenario("lusamine-single-live-crispin"), duplicates);
  expect(EngineTestAccess::play_lusamine(duplicate_fixture.engine),
         "Lusamine should recover a live Energy connector.");
  const State& after_duplicates = EngineTestAccess::state(duplicate_fixture.engine);
  expect(count(after_duplicates.hand, Card::Crispin) == 1 && contains(after_duplicates.hand, Card::PathToPeak),
         "Lusamine must recover a second eligible card after the live Crispin target.");

  State stadiums = state(2, Card::RegidragoV, 1);
  stadiums.hand = {Card::Lusamine};
  stadiums.discard = {Card::PathToPeak, Card::ChaoticSwell};
  Fixture stadium_fixture(scenario("lusamine-dead-stadiums"), stadiums);
  expect(!EngineTestAccess::play_lusamine(stadium_fixture.engine),
         "Lusamine should not spend the Supporter turn only to recover non-setup Stadiums.");
  const State& after_stadiums = EngineTestAccess::state(stadium_fixture.engine);
  expect(contains(after_stadiums.hand, Card::Lusamine),
         "A dead Lusamine line must leave Lusamine in hand.");
}

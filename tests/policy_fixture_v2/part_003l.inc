
void test_fss_is_blocked_by_rule_box_lock() {
  State s = state(2, Card::RegidragoV, 1, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::RegidragoVstar};
  // Forest Seal Stone gives its attached Pokémon V access to the VSTAR Power
  // printed on the Tool: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Path to the Peak removes Abilities from Rule Box Pokémon, rather than
  // the Tool's printed VSTAR Power: https://api.pokemontcg.io/v2/cards/swsh6-148
  Fixture f(scenario("fss-rulebox-lock", DciProfile::StrictJit, LockMode::FullRuleBoxAbility), s);

  expect(EngineTestAccess::use_fss(f.engine),
         "Star Alchemy on an already attached Forest Seal Stone should remain usable under Rule Box lock.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(after.vstar_power_used && contains(after.hand, Card::RegidragoVstar),
         "Star Alchemy should consume the VSTAR Power and search the missing Regidrago VSTAR.");
}

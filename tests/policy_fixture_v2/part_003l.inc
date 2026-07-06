
void test_fss_is_blocked_by_rule_box_lock() {
  State s = state(2, Card::RegidragoV, 1, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::RegidragoVstar};
  Fixture f(scenario("fss-rulebox-lock", DciProfile::StrictJit, LockMode::FullRuleBoxAbility), s);

  expect(!EngineTestAccess::use_fss(f.engine),
         "Path-style Rule Box lock must suppress Star Alchemy granted to Regidrago V.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(!after.vstar_power_used && contains(after.deck, Card::RegidragoVstar),
         "Blocked Star Alchemy must not spend the VSTAR Power or search the deck.");
}

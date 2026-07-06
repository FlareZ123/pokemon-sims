
void test_fss_reveal_preserves_burnet_over_prized_energy() {
  State s = state(2, Card::RegidragoV, 1, 2, 0);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::Gladion, Card::ProfessorBurnet, Card::RegidragoVstar};
  s.deck = {Card::MegaDragonite};
  s.prizes = {Card::Fire};
  Fixture f(scenario("prized-fire-versus-burnet", DciProfile::StrictJit, LockMode::FullItem), s);

  expect(EngineTestAccess::use_fss(f.engine),
         "Already-attached Forest Seal Stone should legally inspect the deck during Item lock in this model.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::ProfessorBurnet) && contains(after.discard, Card::MegaDragonite),
         "Professor Burnet must beat Gladion when Item lock makes it the only payload bridge.");
  expect(contains(after.hand, Card::Gladion),
         "Gladion should be preserved when final Energy alone cannot finish readiness.");
}

void test_heavy_ball_oricorio_preserves_burnet_supporter() {
  State s = state(2, Card::RegidragoVstar, 1, 2, 0);
  s.hand = {Card::HisuianHeavyBall, Card::ProfessorBurnet};
  s.deck = {Card::Fire, Card::MegaDragonite, Card::Crispin, Card::Grass};
  s.prizes = {Card::TapuLeleGX, Card::Oricorio};
  Fixture f(scenario("heavy-ball-oricorio-over-tapu"), s);

  expect(EngineTestAccess::play_heavy_ball(f.engine),
         "Heavy Ball should inspect the two live prized Basic connectors while Items are legal.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Oricorio),
         "When Burnet is the live payload Supporter, Heavy Ball should recover Oricorio over Tapu Lele-GX.");
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::Oricorio, true),
         "The recovered Oricorio should Bench and use Vital Dance.");
  expect(EngineTestAccess::attach_manual(f.engine),
         "The searched Fire should be manually attached to finish GGF.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::ProfessorBurnet) && contains(after.discard, Card::MegaDragonite),
         "Preserved Burnet should discard the Dragon payload in the same turn.");
  expect(after.active->fire == 1,
         "The Oricorio/manual line should complete the final Fire attachment.");
}

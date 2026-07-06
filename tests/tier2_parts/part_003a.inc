void known_lusamine_is_delayed_not_direct() {
  State s = st(2, Card::RegidragoV, 1, 1, 0);
  s.hand = {Card::RegidragoVstar, Card::MysteriousTreasure, Card::Dipplin, Card::Gladion};
  s.deck = {Card::TapuLeleGX};
  s.prizes = {Card::Lusamine};
  s.discard = {Card::Crispin, Card::ProfessorBurnet};
  Fixture f(sc("tier2-lusamine"), s);
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, false), "A legal search should establish K1 before planning a Lusamine line.");
  EngineTestAccess::choose_supporter(f.engine);
  State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::Lusamine), "Known prize Lusamine should be taken only when no direct Supporter route exists.");
  a.turn = 3;
  a.supporter_used = false;
  a.turn_ended = false;
  EngineTestAccess::choose_supporter(f.engine);
  expect(has(a.hand, Card::Crispin) && has(a.hand, Card::ProfessorBurnet), "Lusamine should recover the delayed Supporter package.");
}

void search_effects_persist_knowledge() {
  State quick = st(2, Card::LatiasEx);
  quick.hand = {Card::QuickBall, Card::Dipplin};
  quick.deck = {Card::RegidragoV};
  Fixture q(sc("tier2-quickball-k1"), quick);
  expect(EngineTestAccess::play_quick_ball(q.engine, false) && EngineTestAccess::prizes_known(q.engine), "Quick Ball should establish K1.");

  State heavy = st(2, Card::LatiasEx);
  heavy.hand = {Card::HisuianHeavyBall};
  heavy.prizes = {Card::RegidragoV};
  Fixture h(sc("tier2-heavyball-k1"), heavy);
  expect(EngineTestAccess::play_heavy_ball(h.engine) && EngineTestAccess::prizes_known(h.engine), "Heavy Ball reveal should establish K1.");
}

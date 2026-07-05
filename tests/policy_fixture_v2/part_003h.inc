
void test_heavy_ball_inspects_prizes_and_chains_tapu() {
  State s = state(2);
  s.hand = {Card::HisuianHeavyBall, Card::QuickBall, Card::Dipplin};
  s.prizes = {Card::TapuLeleGX};
  s.deck = {Card::Crispin, Card::Grass, Card::Fire};
  Fixture f(scenario("heavy-ball-tapu-chain"), s);

  expect(EngineTestAccess::play_heavy_ball(f.engine),
         "At K0 Heavy Ball should inspect Prizes even while Quick Ball is live.");
  const State& after_ball = EngineTestAccess::state(f.engine);
  expect(contains(after_ball.hand, Card::TapuLeleGX) && contains(after_ball.prizes, Card::HisuianHeavyBall),
         "Heavy Ball should exchange itself for prized Tapu Lele-GX.");
  expect(contains(after_ball.hand, Card::QuickBall), "Quick Ball should remain available after the Heavy Ball information action.");

  EngineTestAccess::run_search_items_one_step(f.engine, false);
  const State& after_tapu = EngineTestAccess::state(f.engine);
  expect(std::any_of(after_tapu.bench.begin(), after_tapu.bench.end(),
                     [](const Pokemon& p) { return p.card == Card::TapuLeleGX; }),
         "The recovered Tapu Lele-GX should be Benched during the next stabilization step.");
  expect(contains(after_tapu.hand, Card::Crispin),
         "Wonder Tag should chain from the Heavy Ball Tapu into the needed Supporter.");

  EngineTestAccess::choose_supporter(f.engine);
  expect(contains(EngineTestAccess::state(f.engine).discard, Card::Crispin),
         "The chained Crispin should be selected as the turn's Supporter.");
}

void test_known_no_basic_prize_holds_heavy_ball_for_discard_cost() {
  State s = state(2, Card::RegidragoV);
  s.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::HisuianHeavyBall, Card::QuickBall};
  s.deck = {Card::RegidragoVstar, Card::Oricorio, Card::Grass, Card::Fire};
  s.prizes = {Card::Grass, Card::Fire, Card::Dipplin};
  Fixture f(scenario("heavy-ball-known-miss"), s);

  expect(EngineTestAccess::play_mysterious_treasure(f.engine, false),
         "Mysterious Treasure should legally inspect the deck and establish K1.");
  expect(!EngineTestAccess::play_heavy_ball(f.engine),
         "Known Prize cards with no Basic Pokémon should make Heavy Ball stay in hand.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::HisuianHeavyBall),
         "Known-miss Heavy Ball should remain available as a legal discard cost.");

  expect(EngineTestAccess::play_quick_ball(f.engine, false),
         "Quick Ball should use the retained Heavy Ball as its only legal discard cost.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::HisuianHeavyBall) && contains(after.hand, Card::Oricorio),
         "The retained Heavy Ball should become the Quick Ball cost and retrieve Oricorio.");
}

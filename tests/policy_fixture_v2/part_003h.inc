
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

void test_k1_gladion_recovers_final_prized_energy() {
  State s = state(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::Gladion, Card::Crispin};
  s.deck = {Card::RegidragoVstar};
  s.prizes = {Card::Fire};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("k1-gladion-prized-fire"), s);

  expect(EngineTestAccess::play_mysterious_treasure(f.engine, false),
         "Mysterious Treasure should establish K1 before Gladion is evaluated.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::RegidragoVstar),
         "The legal Treasure search should take the missing VSTAR and expose the empty Fire deck axis.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Fire),
         "Gladion should take the known prized final Fire Energy instead of deferring to dead Crispin.");
  expect(EngineTestAccess::attach_manual(f.engine),
         "The final prized Fire should be manually attachable this turn.");
  expect(EngineTestAccess::state(f.engine).active->fire == 1,
         "The manual attachment should complete the Fire part of GGF.");
}

void test_heavy_ball_reveal_marks_crispin_dead() {
  State s = state(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::HisuianHeavyBall, Card::Crispin, Card::Gladion};
  s.deck = {Card::RegidragoVstar};
  s.prizes = {Card::Fire};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("heavy-ball-revealed-prized-fire"), s);

  expect(EngineTestAccess::play_heavy_ball(f.engine),
         "Heavy Ball should be played at K0 to reveal the Prize state.");
  expect(contains(EngineTestAccess::state(f.engine).discard, Card::HisuianHeavyBall),
         "With no Prize Basic, Heavy Ball should be discarded by its printed miss clause.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Fire),
         "The revealed Prize Fire should make Gladion beat a Crispin known unable to find Fire.");
}


void test_celestial_roar_attached_energy_leaves_discard() {
  State s = state(1, Card::RegidragoV, 0, 1, 0);
  s.deck = {Card::Dipplin, Card::Fire, Card::Grass};
  Fixture f(scenario("celestial-roar-zone-transfer"), s);

  expect(EngineTestAccess::use_celestial_roar(f.engine),
         "A going-second Regidrago V with one attached Energy should be able to use Celestial Roar.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(after.active->grass == 2 && after.active->fire == 1,
         "Grass and Fire found among the top three should be attached to Regidrago V.");
  expect(!contains(after.discard, Card::Grass) && !contains(after.discard, Card::Fire),
         "Attached Energy cards must not remain in the discard pile.");
  expect(contains(after.discard, Card::Dipplin),
         "Only the non-Energy top-three card should remain discarded.");
}

void test_tate_draws_from_a_four_card_dead_hand() {
  State s = state(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TateLiza, Card::Dipplin, Card::MawileGX, Card::Guzma};
  s.deck = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("tate-four-card-draw"), s);

  EngineTestAccess::choose_supporter(f.engine);
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::TateLiza),
         "Tate and Liza should be used from a four-card dead hand because draw-five is net positive.");
  expect(contains(after.hand, Card::RegidragoVstar),
         "The draw-five resolution should find the only VSTAR in the compact deck.");
}

void test_fss_fetches_gladion_for_final_prized_energy() {
  State s = state(2, Card::RegidragoV, 1, 2, 0);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::RegidragoVstar};
  s.deck = {Card::Gladion};
  s.prizes = {Card::Fire};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(scenario("fss-gladion-prized-fire"), s);

  expect(EngineTestAccess::use_fss(f.engine),
         "Star Alchemy should inspect the deck and fetch Gladion for the known Prize Energy route.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Gladion),
         "Star Alchemy should fetch Gladion rather than a slower connector.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Fire),
         "Gladion should recover the prized Fire after Star Alchemy establishes K1.");
}

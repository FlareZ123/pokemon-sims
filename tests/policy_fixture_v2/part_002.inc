  s.deck = {Card::Grass, Card::Grass};
  Fixture f(scenario("vessel-two-grass"), s);
  expect(EngineTestAccess::play_earthen_vessel(f.engine, false), "Earthen Vessel should play with a dead Dipplin discard cost.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(count(after.hand, Card::Grass) == 2, "Earthen Vessel should select two Grass Energy when both Grass attachments remain.");
}

void test_strict_jit_holds_payload_before_turn_two() {
  State s = state(1);
  s.hand = {Card::MysteriousTreasure, Card::MegaDragonite};
  s.deck = {Card::RegidragoVstar};
  Fixture strict(scenario("strict-t1"), s);
  expect(!EngineTestAccess::play_mysterious_treasure(strict.engine, true), "Strict JIT must not discard the only payload before turn two.");
  expect(contains(EngineTestAccess::state(strict.engine).hand, Card::MegaDragonite), "The protected payload must remain in hand on strict turn one.");

  s.turn = 2;
  Fixture turn_two(scenario("strict-t2"), s);
  expect(EngineTestAccess::play_mysterious_treasure(turn_two.engine, true), "The same payload discard becomes legal in the strict ready window on turn two.");
  expect(contains(EngineTestAccess::state(turn_two.engine).discard, Card::MegaDragonite), "Turn-two Mysterious Treasure should discard the payload.");
}

void test_no_control_allows_early_payload_banking() {
  State s = state(1, Card::LatiasEx);
  s.hand = {Card::QuickBall, Card::MegaDragonite};
  s.deck = {Card::RegidragoV};
  Fixture strict(scenario("strict-no-bank"), s);
  expect(!EngineTestAccess::play_quick_ball(strict.engine, true), "Strict JIT should not spend the only payload on turn one.");
  Fixture uncontrolled(scenario("no-control-bank", DciProfile::NoDiscardControl), s);
  expect(EngineTestAccess::play_quick_ball(uncontrolled.engine, true), "No-discard-control may bank payload through Quick Ball on turn one.");
  expect(contains(EngineTestAccess::state(uncontrolled.engine).discard, Card::MegaDragonite), "The early payload must be in discard in the no-control profile.");
}

void test_blender_dominates_mysterious_treasure_when_no_search_axis_exists() {
  State s = state(2, Card::RegidragoVstar, 0, 2, 1);
  s.hand = {Card::BrilliantBlender, Card::MysteriousTreasure, Card::Dipplin};
  s.deck = {Card::MegaDragonite};
  Fixture f(scenario("blender-first"), s);
  expect(EngineTestAccess::run_search_items_one_step(f.engine, true), "The item policy should take a payload outlet step.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.discard, Card::BrilliantBlender) && contains(after.discard, Card::MegaDragonite),
         "Brilliant Blender should supply the payload before using Mysterious Treasure.");
  expect(contains(after.hand, Card::MysteriousTreasure), "Mysterious Treasure should be preserved when it does not solve another axis.");
}

void test_searches_use_legal_fallback_targets_after_deck_inspection() {
  State s = state(2, Card::LatiasEx);
  s.hand = {Card::QuickBall, Card::Dipplin};
  s.deck = {Card::TapuLeleGX};
  Fixture f(scenario("quickball-fallback"), s);
  expect(EngineTestAccess::play_quick_ball(f.engine, false), "Quick Ball should resolve even when Regidrago V is absent from deck.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::TapuLeleGX), "Quick Ball should select a legal fallback Basic after search starts.");
}


void test_oricorio_is_held_when_earthen_vessel_already_solves_energy() {
  State s = state(2);
  s.hand = {Card::Oricorio, Card::EarthenVessel, Card::Dipplin};
  s.deck = {Card::Grass, Card::Fire};
  Fixture f(scenario("oricorio-held"), s);
  expect(!EngineTestAccess::bench_oricorio_if_useful(f.engine), "Oricorio should be held when Earthen Vessel already covers the Energy axis.");
  expect(EngineTestAccess::play_earthen_vessel(f.engine, false), "Earthen Vessel should be the direct connector selected instead.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Oricorio), "Held Oricorio should remain available for a later turn.");
}

void test_oricorio_works_under_rule_box_lock() {
  State s = state(2);
  s.hand = {Card::Oricorio};
  s.deck = {Card::Grass, Card::Fire};
  Fixture f(scenario("oricorio-path", DciProfile::StrictJit, LockMode::FullRuleBoxAbility), s);
  expect(EngineTestAccess::bench_oricorio_if_useful(f.engine), "Oricorio should be benched as the live Energy connector.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::Grass) && contains(after.hand, Card::Fire), "Oricorio Vital Dance must remain active through Rule Box lock.");
}

void test_tapu_is_blocked_by_rule_box_lock() {
  State s = state(1);
  s.hand = {Card::TapuLeleGX};
  s.deck = {Card::Crispin};
  Fixture f(scenario("tapu-path", DciProfile::StrictJit, LockMode::FullRuleBoxAbility), s);
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true), "Tapu Lele-GX may still be benched under the lock.");
  expect(!contains(EngineTestAccess::state(f.engine).hand, Card::Crispin), "Wonder Tag must be blocked when Rule Box abilities are locked.");
}

void test_tapu_fetches_steven_when_it_is_the_live_turn_one_connector() {
  State s = state(1);
  s.hand = {Card::TapuLeleGX};

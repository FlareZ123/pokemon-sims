
void expect_issue_4097_burnet_latias_lock_case(const LockMode locks,
                                                const bool path_lock_removed,
                                                const bool expect_burnet) {
  State s = state(3, Card::TapuLeleGX, 0, 0, 0);
  s.path_lock_removed = path_lock_removed;
  s.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::ForestSealStone}};
  s.hand = {Card::TapuLeleGX, Card::QuickBall, Card::StevensResolve, Card::Fire};
  s.deck = {Card::ProfessorBurnet, Card::Crispin, Card::LatiasEx,
            Card::MegaDragonite, Card::Grass, Card::Grass, Card::Fire};
  Fixture f(scenario("issue-4097-burnet-latias-lock", DciProfile::StrictJit,
                     locks, false),
            s);
  EngineTestAccess::set_deck_seen(f.engine, true);

  // This exact #2343 continuation uses Wonder Tag, Professor Burnet, Quick Ball,
  // Latias ex's Skyliner, one manual Fire attachment, Retreat, and Apex Dragon.
  // Each lock therefore applies to its own action class, while a removed Path-style
  // Ability lock must stop suppressing Skyliner without erasing historical scenario identity:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // https://github.com/FlareZ123/pokemon-sims/issues/2343
  // https://github.com/FlareZ123/pokemon-sims/issues/4097
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true),
         "The fixture must Bench the second Tapu Lele-GX and resolve Wonder Tag.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::ProfessorBurnet) ==
             expect_burnet,
         expect_burnet
             ? "Recovered Path must admit Wonder Tag -> Burnet -> Quick Ball -> Latias ex."
             : "A live Ability, Item, Supporter, or combined blocker must keep the #2343 Burnet route unavailable.");
}

void test_issue_4097_burnet_latias_uses_action_specific_locks() {
  expect_issue_4097_burnet_latias_lock_case(LockMode::None, false, true);
  expect_issue_4097_burnet_latias_lock_case(LockMode::FullRuleBoxAbility, true, true);
  expect_issue_4097_burnet_latias_lock_case(LockMode::FullRuleBoxAbility, false, false);
  expect_issue_4097_burnet_latias_lock_case(LockMode::FullItem, false, false);
  expect_issue_4097_burnet_latias_lock_case(LockMode::TurnTwoItem, false, false);
  expect_issue_4097_burnet_latias_lock_case(LockMode::FullCombined, true, false);
  expect_issue_4097_burnet_latias_lock_case(LockMode::FullSupporter, false, false);
}

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

  test_issue_4097_burnet_latias_uses_action_specific_locks();
}

void expect_issue_4093_wonder_tag_lock_case(const LockMode locks,
                                             const bool path_lock_removed,
                                             const bool expect_steven) {
  State s = state(1, Card::RegidragoV, 0, 0, 0);
  s.path_lock_removed = path_lock_removed;
  s.hand = {Card::TapuLeleGX, Card::RegidragoVstar, Card::Grass, Card::Grass};
  s.deck = {Card::StevensResolve, Card::Crispin, Card::EarthenVessel,
            Card::Grass, Card::Grass, Card::Grass, Card::Grass, Card::Fire,
            Card::MegaDragonite};
  s.discard = {Card::QuickBall, Card::TateLiza};
  Fixture f(scenario("issue-4093-wonder-tag-lock", DciProfile::StrictJit,
                     locks, true),
            s);

  // Wonder Tag is a Rule Box Pokémon Ability, Quick Ball and Earthen Vessel are Items,
  // and Steven's Resolve plus Crispin are Supporters. Their legality therefore has
  // to be projected by action class across this route instead of by aggregate lock identity:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true),
         "The fixture must be able to Bench Tapu Lele-GX and resolve its entry event.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::StevensResolve) ==
             expect_steven,
         expect_steven
             ? "Recovered Path must admit the legal T1 Wonder Tag -> T2 Steven -> T3 Crispin/Vessel route."
             : "A route-blocking Ability, Item, or Supporter restriction must keep Steven out of the hand.");
}

void test_issue_4093_wonder_tag_uses_action_specific_lock_projection() {
  expect_issue_4093_wonder_tag_lock_case(LockMode::None, false, true);
  expect_issue_4093_wonder_tag_lock_case(LockMode::FullRuleBoxAbility, true, true);
  expect_issue_4093_wonder_tag_lock_case(LockMode::FullRuleBoxAbility, false, false);
  expect_issue_4093_wonder_tag_lock_case(LockMode::FullItem, false, false);
  expect_issue_4093_wonder_tag_lock_case(LockMode::TurnTwoItem, false, false);
  expect_issue_4093_wonder_tag_lock_case(LockMode::FullCombined, true, false);
  expect_issue_4093_wonder_tag_lock_case(LockMode::FullSupporter, false, false);
}

void expect_issue_4108_crispin_replacement_lock_case(const LockMode locks,
                                                      const bool path_lock_removed,
                                                      const bool expect_grass_cost) {
  State s = state(1, Card::Oricorio, 0, 0, 0);
  s.path_lock_removed = path_lock_removed;
  s.bench = {Pokemon{Card::LatiasEx, 0, 0, 0, Tool::None}};
  s.hand = {Card::QuickBall, Card::Crispin, Card::Grass, Card::Grass,
            Card::Arven, Card::BrilliantBlender};
  s.deck = {Card::RegidragoV, Card::Grass, Card::Fire, Card::ForestSealStone,
            Card::RegidragoVstar, Card::MegaDragonite};
  Fixture f(scenario("issue-4108-crispin-lock", DciProfile::StrictJit, locks), s);
  EngineTestAccess::set_deck_seen(f.engine, true);

  // This exact strict-JIT route needs the current Crispin Supporter, projected
  // Brilliant Blender Item, and Latias ex's Skyliner Ability. Recovered Path restores
  // Skyliner, while live Rule Box Ability lock, Item lock, T2 Item lock, Supporter
  // lock, and combined lock each block a required action at its real timing point.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Advanced action procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4108
  const auto cost = EngineTestAccess::quick_ball_crispin_replaced_energy_cost(f.engine);
  expect((cost && *cost == Card::Grass) == expect_grass_cost,
         expect_grass_cost
             ? "Unlocked and recovered-Path states must admit the Grass replacement cost."
             : "A live restriction on any required route action must reject the Grass replacement cost.");
}

void test_issue_4108_crispin_replacement_uses_action_specific_locks() {
  expect_issue_4108_crispin_replacement_lock_case(LockMode::None, false, true);
  expect_issue_4108_crispin_replacement_lock_case(LockMode::FullRuleBoxAbility, true, true);
  expect_issue_4108_crispin_replacement_lock_case(LockMode::FullRuleBoxAbility, false, false);
  expect_issue_4108_crispin_replacement_lock_case(LockMode::FullItem, false, false);
  expect_issue_4108_crispin_replacement_lock_case(LockMode::TurnTwoItem, false, false);
  expect_issue_4108_crispin_replacement_lock_case(LockMode::FullCombined, true, false);
  expect_issue_4108_crispin_replacement_lock_case(LockMode::FullSupporter, false, false);
}

// The fixture registry retains its historical symbol while this file also hosts
// the route-lock regressions above.
void test_arven_is_not_played_during_item_lock() {
  State s = state(2);
  s.hand = {Card::Arven};
  s.deck = {Card::RegidragoVstar, Card::ForestSealStone};
  Fixture f(scenario("arven-item-lock", DciProfile::StrictJit, LockMode::FullItem), s);

  // Arven is a Supporter, so Item lock does not prohibit playing it; Pokémon Tools
  // remain separate from Items after the Tool erratum: https://api.pokemontcg.io/v2/cards/sv1-166 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  expect(EngineTestAccess::play_arven(f.engine), "Arven should use the live Forest Seal Stone Tool route during Item lock.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::ForestSealStone) && after.supporter_used,
         "Arven should fetch Forest Seal Stone and consume the Supporter play.");

  test_issue_4093_wonder_tag_uses_action_specific_lock_projection();
  test_issue_4108_crispin_replacement_uses_action_specific_locks();
}

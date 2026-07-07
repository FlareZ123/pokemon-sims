void tapu_avoids_redundant_arven() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TapuLeleGX, Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tapu-vstar-in-hand"), s);
  expect(!EngineTestAccess::bench_tapu_if_useful(f.engine), "Tapu should remain unused.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::TapuLeleGX), "Tapu should remain in hand.");
}

void tapu_uses_gladion_for_known_prized_vstar() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TapuLeleGX, Card::Arven, Card::Crispin};
  s.deck = {Card::Gladion, Card::Arven, Card::Crispin};
  s.prizes = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tapu-known-prized-vstar"), s);
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true), "Tapu should enter the Bench and resolve Wonder Tag.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Gladion), "Wonder Tag should select Gladion after deck inspection.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::RegidragoVstar), "Gladion should recover the known prized VSTAR.");
  expect(has(a.hand, Card::Arven) && has(a.hand, Card::Crispin), "The direct Gladion route should preserve slower Supporters.");
}

void crispin_beats_known_prized_oricorio() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::MysteriousTreasure, Card::MegaDragonite, Card::Gladion, Card::Crispin, Card::RegidragoVstar};
  s.deck = {Card::TapuLeleGX, Card::Grass, Card::Fire};
  s.prizes = {Card::Oricorio};
  Fixture f(sc("crispin-over-prized-oricorio"), s);
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, true), "Mysterious Treasure should establish K1 legally.");
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::Crispin), "Crispin should beat a known prized Oricorio connector.");
  expect(has(a.hand, Card::Gladion), "Gladion should remain available after the direct Energy line.");
  expect(a.active->fire == 1, "Crispin should attach the missing Fire immediately.");
}

void fss_holds_for_tate_switch() {
  State s = st(2, Card::RegidragoV, 1);
  s.active->tool = Tool::ForestSealStone;
  s.bench.push_back(Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None});
  s.hand = {Card::TateLiza};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("fss-hold-for-tate"), s);
  expect(!EngineTestAccess::use_fss(f.engine), "FSS should be preserved when Tate and Liza alone fixes Active positioning.");
  expect(!EngineTestAccess::state(f.engine).vstar_power_used, "The VSTAR Power should remain unused.");
}

void fss_uses_gladion_for_known_prized_vstar() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::Gladion, Card::Arven, Card::Crispin};
  s.prizes = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("fss-known-prized-vstar"), s);
  expect(EngineTestAccess::use_fss(f.engine), "FSS should resolve as a legal deck inspection.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Gladion), "FSS should select Gladion for the known prized VSTAR.");
  EngineTestAccess::choose_supporter(f.engine);
  expect(has(EngineTestAccess::state(f.engine).hand, Card::RegidragoVstar), "Gladion should recover the prized VSTAR after FSS inspection.");
}

void heavy_ball_oricorio_compresses_energy() {
  State s = st(2, Card::RegidragoV, 1);
  s.hand = {Card::HisuianHeavyBall};
  s.prizes = {Card::Oricorio};
  s.deck = {Card::Grass, Card::Fire};
  Fixture f(sc("heavy-ball-oricorio"), s);
  expect(EngineTestAccess::play_heavy_ball(f.engine), "Heavy Ball should retrieve prized Oricorio when it is the direct Energy connector.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Oricorio), "Heavy Ball should put Oricorio into hand.");
  expect(EngineTestAccess::bench_from_hand(f.engine, Card::Oricorio, true), "Oricorio should be benched and resolve Vital Dance.");
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.hand, Card::Grass) && has(a.hand, Card::Fire), "Vital Dance should supply both available needed Energy cards.");
}

void ability_lock_blocks_latias_quick_ball_connector() {
  State s = st(2, Card::RegidragoV, 1);
  s.bench.push_back(Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None});
  s.hand = {Card::QuickBall, Card::Dipplin};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("locked-latias-quick-ball", DciProfile::StrictJit, LockMode::FullRuleBoxAbility), s);
  expect(!EngineTestAccess::play_quick_ball(f.engine, false), "Quick Ball must not be spent to seek Latias when Skyliner is Ability-locked.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::QuickBall), "The unused Quick Ball should remain in hand.");
}

void fss_is_blocked_by_rulebox_lock() {
  State s = st(2, Card::RegidragoV, 1);
  s.active->tool = Tool::ForestSealStone;
  s.bench.push_back(Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None});
  s.deck = {Card::TateLiza};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  // Forest Seal Stone lets the attached Pokémon V use the VSTAR Power printed
  // on the Tool: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Path to the Peak suppresses Abilities on Rule Box Pokémon, not the Tool's
  // printed VSTAR Power: https://api.pokemontcg.io/v2/cards/swsh6-148
  Fixture f(sc("locked-latias-fss-tate", DciProfile::StrictJit, LockMode::FullRuleBoxAbility), s);
  expect(EngineTestAccess::use_fss(f.engine),
         "An already attached Forest Seal Stone should remain usable under Rule Box lock.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::TateLiza),
         "Star Alchemy should search Tate and Liza to repair the Active-position axis.");
}

void k1_heavy_ball_beats_still_live_quick_ball() {
  State s = st(2, Card::RegidragoV, 1);
  s.hand = {Card::MysteriousTreasure, Card::MegaDragonite, Card::RegidragoVstar, Card::HisuianHeavyBall, Card::QuickBall};
  s.deck = {Card::TapuLeleGX, Card::Grass, Card::Fire};
  s.prizes = {Card::Oricorio};
  Fixture f(sc("k1-heavy-ball-over-quick-ball"), s);
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, true), "Mysterious Treasure should legally create K1.");
  expect(EngineTestAccess::prizes_known(f.engine), "The deck search should reveal that Oricorio is prized.");
  expect(EngineTestAccess::play_heavy_ball(f.engine), "Known prized Oricorio should make Heavy Ball the direct connector despite Quick Ball in hand.");
  expect(has(EngineTestAccess::state(f.engine).hand, Card::Oricorio), "Heavy Ball should recover the known prized Oricorio.");
}

void tate_draw_recovers_missing_vstar() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::TateLiza};
  s.deck = {Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("tate-draw-vstar"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::TateLiza), "Tate and Liza should be used in draw mode when no direct Supporter line exists.");
  expect(has(a.hand, Card::RegidragoVstar), "Tate and Liza should draw the missing VSTAR from the compact deck.");
}

void arven_uses_available_vstar_item_fallback() {
  State s = st(2, Card::RegidragoV, 1, 2, 1);
  s.hand = {Card::Arven, Card::Dipplin};
  s.deck = {Card::MysteriousTreasure, Card::ForestSealStone, Card::RegidragoVstar};
  s.discard = {Card::MegaDragonite};
  s.discarded_this_turn = {Card::MegaDragonite};
  Fixture f(sc("arven-vstar-item-fallback"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::Arven), "Arven should be the selected Supporter.");
  expect(has(a.hand, Card::MysteriousTreasure), "Arven should take Mysterious Treasure when Evolution Incense is unavailable.");
  expect(has(a.hand, Card::ForestSealStone), "Arven should still take the available Tool.");
}

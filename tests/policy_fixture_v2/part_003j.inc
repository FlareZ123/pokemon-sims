
void test_fss_reveal_preserves_burnet_over_prized_energy() {
  State s = state(2, Card::RegidragoV, 1, 2, 0);
  s.active->tool = Tool::ForestSealStone;
  s.hand = {Card::Gladion, Card::ProfessorBurnet, Card::RegidragoVstar};
  s.deck = {Card::MegaDragonite};
  s.prizes = {Card::Fire};
  Fixture f(scenario("prized-fire-versus-burnet", DciProfile::StrictJit, LockMode::FullItem), s);
  // This fixture models K1 knowledge after a legal deck search: the singleton deck
  // complement proves that Fire is the remaining Prize card.
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  EngineTestAccess::set_deck_seen(f.engine, true);

  // Star Alchemy must search for a card because its text gives no choice. With the
  // sole deck card as Burnet's payload, using FSS would remove that target:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/2-meta-rulings/
  expect(!EngineTestAccess::use_fss(f.engine),
         "Forest Seal Stone must be held when its mandatory search would empty Burnet's only payload zone.");

  // Strict JIT accepts a Dragon only when it enters discard during the present ready
  // turn. Burnet cannot create readiness while the needed Energy is still a Prize
  // because the one Supporter play would then be unavailable for Gladion:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  EngineTestAccess::choose_supporter(f.engine);
  const State& after_supporter = EngineTestAccess::state(f.engine);
  expect(contains(after_supporter.hand, Card::Fire) &&
             contains(after_supporter.hand, Card::ProfessorBurnet) &&
             contains(after_supporter.deck, Card::MegaDragonite),
         "Gladion should recover the known prized Energy while Burnet and its payload remain uncommitted.");
  expect(contains(after_supporter.prizes, Card::Gladion) &&
             !after_supporter.vstar_power_used,
         "Gladion should exchange into the Prizes while Forest Seal Stone stays unused.");
  expect(!contains(after_supporter.discard, Card::ProfessorBurnet) &&
             !contains(after_supporter.discard, Card::MegaDragonite),
         "Burnet must not create an expiring strict-JIT payload before the Energy axis can finish.");

  expect(EngineTestAccess::attach_manual(f.engine),
         "The recovered Energy should complete the prior-turn Regidrago V's attack cost.");
  expect(EngineTestAccess::evolve_best_regi(f.engine),
         "The prior-turn Regidrago V should evolve after the Energy recovery Supporter resolves.");
  // Legacy Star discards the top 7 cards, so the known singleton Mega Dragonite ex
  // becomes the same-turn Dragon payload after Gladion finishes the Energy axis:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect(EngineTestAccess::use_legacy_star(f.engine),
         "Legacy Star should discard the known singleton Dragon payload in the same turn.");
  expect(EngineTestAccess::payload_ready(f.engine),
         "Gladion into Legacy Star should complete the strict-JIT ready state.");
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

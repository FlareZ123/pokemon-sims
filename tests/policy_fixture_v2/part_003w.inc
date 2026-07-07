void test_evolution_incense_is_held_when_k1_proves_vstar_prized() {
  State s = state(2, Card::RegidragoV, 0, 2, 1);
  s.hand = {Card::HisuianHeavyBall, Card::EvolutionIncense};
  s.prizes = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::MawileGX, Card::Guzma};
  s.deck = {Card::GoodraVstar};
  // Hisuian Heavy Ball legally reveals the Prize cards: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Evolution Incense searches an Evolution Pokémon: https://api.pokemontcg.io/v2/cards/swsh1-163
  Fixture f(scenario("incense-known-prized-vstar"), s);
  expect(EngineTestAccess::play_heavy_ball(f.engine), "Heavy Ball should establish K1 through its required Prize inspection.");
  expect(!EngineTestAccess::run_search_items_one_step(f.engine, false),
         "Evolution Incense must be held when K1 proves Regidrago VSTAR is prized.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::EvolutionIncense) && !contains(after.hand, Card::GoodraVstar),
         "The Item must not be spent to fetch an unrelated Evolution that cannot fill the VSTAR axis.");
}

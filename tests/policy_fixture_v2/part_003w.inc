// The fixture registry retains its historical symbol while the test itself now
// validates Arven's legal Forest Seal Stone route under Item lock.
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
}

void test_payload_selectors_fetch_burnet_when_blender_is_unavailable() {
  // Professor Burnet searches the deck and discards up to 2 cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  {
    State s = state(2, Card::RegidragoVstar, 0, 2, 1);
    s.hand = {Card::TapuLeleGX};
    s.deck = {Card::ProfessorBurnet, Card::MegaDragonite};
    Fixture f(scenario("tapu-burnet"), s);
    expect(EngineTestAccess::bench_from_hand(f.engine, Card::TapuLeleGX, true), "Tapu Lele-GX should resolve Wonder Tag.");
    expect(contains(EngineTestAccess::state(f.engine).hand, Card::ProfessorBurnet), "Wonder Tag should take Burnet when Blender cannot supply the payload route.");
  }
  {
    State s = state(2, Card::RegidragoV, 0, 1, 0);
    s.active->tool = Tool::ForestSealStone;
    s.prizes = {Card::RegidragoVstar};
    s.deck = {Card::ProfessorBurnet, Card::Serena};
    Fixture f(scenario("fss-burnet"), s);
    expect(EngineTestAccess::use_fss(f.engine), "Forest Seal Stone should resolve Star Alchemy.");
    expect(contains(EngineTestAccess::state(f.engine).hand, Card::ProfessorBurnet), "Star Alchemy should prefer Burnet over a dead Serena fallback.");
  }
  {
    State s = state(1);
    s.hand = {Card::StevensResolve};
    s.deck = {Card::RegidragoVstar, Card::Crispin, Card::ProfessorBurnet, Card::Grass, Card::Fire};
    Fixture f(scenario("steven-burnet-no-blender"), s);
    expect(EngineTestAccess::play_steven(f.engine), "Steven's Resolve should cover the three missing setup axes.");
    expect(contains(EngineTestAccess::state(f.engine).hand, Card::ProfessorBurnet), "Steven's Resolve should reserve Burnet when Blender is absent from deck.");
  }
}

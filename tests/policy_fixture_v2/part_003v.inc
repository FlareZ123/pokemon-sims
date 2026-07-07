void test_arven_requires_a_live_item_or_tool_route() {
  {
    State s = state(2, Card::RegidragoV, 0, 2, 1);
    s.hand = {Card::Arven, Card::HisuianHeavyBall};
    Fixture f(scenario("arven-known-empty"), s);
    expect(EngineTestAccess::play_heavy_ball(f.engine), "Heavy Ball should establish deck knowledge.");
    expect(!EngineTestAccess::play_arven(f.engine),
           "Arven must remain unplayed when K1 proves no usable Item or Forest Seal Stone remains.");
    const State& after = EngineTestAccess::state(f.engine);
    expect(contains(after.hand, Card::Arven) && !after.supporter_used,
           "A dead Arven route must preserve the Supporter turn.");
  }
  {
    State s = state(2, Card::RegidragoV, 0, 2, 1);
    s.hand = {Card::Arven, Card::HisuianHeavyBall};
    s.deck = {Card::MysteriousTreasure};
    Fixture f(scenario("arven-unpayable-mt"), s);
    expect(EngineTestAccess::play_heavy_ball(f.engine), "Heavy Ball should establish deck knowledge.");
    expect(!EngineTestAccess::play_arven(f.engine),
           "Arven must not take a known Mysterious Treasure when no discard cost is payable.");
  }
  {
    State s = state(2, Card::RegidragoV, 0, 2, 1);
    s.hand = {Card::Arven, Card::HisuianHeavyBall};
    s.deck = {Card::EvolutionIncense};
    Fixture f(scenario("arven-dead-incense-target"), s);
    expect(EngineTestAccess::play_heavy_ball(f.engine), "Heavy Ball should establish deck knowledge.");
    expect(!EngineTestAccess::play_arven(f.engine),
           "Arven must not take a known Evolution Incense when Regidrago VSTAR is absent.");
  }
  {
    State s = state(2, Card::RegidragoVstar, 0, 1, 0);
    s.hand = {Card::Arven, Card::HisuianHeavyBall, Card::Dipplin};
    s.discard = {Card::MegaDragonite};
    s.deck = {Card::EarthenVessel, Card::Fire};
    Fixture f(scenario("arven-live-vessel"), s);
    expect(EngineTestAccess::play_heavy_ball(f.engine), "Heavy Ball should establish deck knowledge.");
    expect(EngineTestAccess::play_arven(f.engine),
           "Arven should play when Earthen Vessel is a known payable Energy route.");
    expect(contains(EngineTestAccess::state(f.engine).hand, Card::EarthenVessel),
           "Arven should retrieve the live Earthen Vessel route.");
  }
  {
    State s = state(2, Card::RegidragoV, 0, 2, 0);
    s.hand = {Card::Arven, Card::Dipplin};
    s.prizes = {Card::RegidragoVstar};
    s.deck = {Card::EvolutionIncense, Card::EarthenVessel, Card::Fire};
    // Arven searches the deck before choosing its Item target: https://api.pokemontcg.io/v2/cards/sv1-166
    Fixture f(scenario("arven-rechecks-inspected-target"), s);
    expect(EngineTestAccess::play_arven(f.engine),
           "Arven should resolve when Earthen Vessel is a live post-search Energy route.");
    const State& after = EngineTestAccess::state(f.engine);
    expect(contains(after.hand, Card::EarthenVessel),
           "After seeing prized VSTAR, Arven must take Earthen Vessel instead of dead Evolution Incense.");
    expect(!contains(after.hand, Card::EvolutionIncense),
           "Arven must not retain a stale K0 Evolution Incense target after legal deck inspection.");
  }
}

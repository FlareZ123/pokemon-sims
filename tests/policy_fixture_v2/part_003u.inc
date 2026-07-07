void test_lusamine_recovers_current_axis_connectors() {
  // Lusamine puts 2 Supporter and/or Stadium cards from discard into hand: https://api.pokemontcg.io/v2/cards/sm4-96
  {
    State s = state(2, Card::RegidragoV, 0, 2, 1);
    s.hand = {Card::Lusamine, Card::Dipplin};
    s.discard = {Card::Crispin, Card::ProfessorBurnet, Card::Arven};
    s.deck = {Card::MysteriousTreasure};
    Fixture f(scenario("lusamine-vstar-axis"), s);
    expect(EngineTestAccess::play_lusamine(f.engine), "Lusamine should recover a live VSTAR connector.");
    const State& after = EngineTestAccess::state(f.engine);
    expect(contains(after.hand, Card::Arven),
           "A missing VSTAR must recover Arven before unrelated Crispin.");
    expect(contains(after.hand, Card::ProfessorBurnet),
           "Lusamine must recover a second eligible Supporter when Professor Burnet is available.");
    expect(count(after.hand, Card::Arven) + count(after.hand, Card::ProfessorBurnet) == 2,
           "Lusamine should return exactly two selected eligible cards when two are available.");
  }
  {
    State s = state(2, Card::TapuLeleGX);
    s.bench = {Pokemon{Card::RegidragoVstar, 0, 2, 1, Tool::None}};
    s.hand = {Card::Lusamine};
    s.discard = {Card::Crispin, Card::TateLiza, Card::MegaDragonite};
    Fixture f(scenario("lusamine-active-axis", DciProfile::NoDiscardControl), s);
    expect(EngineTestAccess::play_lusamine(f.engine), "Lusamine should recover the Active-position connector.");
    const State& after = EngineTestAccess::state(f.engine);
    expect(contains(after.hand, Card::TateLiza),
           "A benched ready VSTAR must recover Tate & Liza for the Active axis.");
    expect(contains(after.hand, Card::Crispin),
           "Lusamine must also recover the remaining eligible Crispin as its mandatory second card.");
  }
  {
    State s = state(2, Card::RegidragoVstar, 0, 1, 1);
    s.hand = {Card::Lusamine};
    s.discard = {Card::Crispin, Card::ProfessorBurnet, Card::MegaDragonite};
    s.deck = {Card::Grass, Card::Fire};
    Fixture f(scenario("lusamine-energy-axis", DciProfile::NoDiscardControl), s);
    expect(EngineTestAccess::play_lusamine(f.engine), "Lusamine should recover the Energy connector.");
    const State& after = EngineTestAccess::state(f.engine);
    expect(contains(after.hand, Card::Crispin),
           "A missing Energy component must recover Crispin.");
    expect(contains(after.hand, Card::ProfessorBurnet),
           "Lusamine must recover Professor Burnet as the second eligible Supporter.");
  }
}

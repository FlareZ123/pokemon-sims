void test_lusamine_recovers_current_axis_connectors() {
  // Lusamine may recover up to two Supporter or Stadium cards: https://api.pokemontcg.io/v2/cards/sm4-96
  {
    State s = state(2, Card::RegidragoV, 0, 2, 1);
    s.hand = {Card::Lusamine};
    s.discard = {Card::Crispin, Card::ProfessorBurnet, Card::Arven};
    s.deck = {Card::EvolutionIncense};
    Fixture f(scenario("lusamine-vstar-axis"), s);
    expect(EngineTestAccess::play_lusamine(f.engine), "Lusamine should recover a live VSTAR connector.");
    const State& after = EngineTestAccess::state(f.engine);
    expect(contains(after.hand, Card::Arven),
           "A missing VSTAR must recover Arven before unrelated Crispin.");
    expect(!contains(after.hand, Card::Crispin),
           "Crispin must not be recovered when the Energy axis is already complete.");
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
    expect(!contains(after.hand, Card::Crispin),
           "Crispin must not outrank Tate & Liza when no Energy is missing.");
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
    expect(!contains(after.hand, Card::ProfessorBurnet),
           "Burnet must not be recovered when a payload is already in discard.");
  }
}

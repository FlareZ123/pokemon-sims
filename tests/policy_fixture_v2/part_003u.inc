void test_lusamine_recovers_current_axis_connectors() {
  // Lusamine may recover up to two Supporter or Stadium cards: https://api.pokemontcg.io/v2/cards/sm4-96
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

void test_k0_counts_public_attached_cards() {
  State s = state(2, Card::RegidragoVstar, 0, 2, 0);
  s.bench = {Pokemon{Card::RegidragoV, 0, 0, 1, Tool::None}};
  s.hand = {Card::Crispin};
  s.discard = {Card::Fire, Card::Fire};
  s.deck = {Card::Grass};
  // All three Fire copies are public: two in discard and one attached to the Bench.
  // Crispin only advances the missing Fire axis if a Fire could still be searched:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  Fixture f(scenario("k0-public-attached-fire", DciProfile::NoDiscardControl), s);
  expect(!EngineTestAccess::play_crispin(f.engine),
         "Crispin must be held when public attached and discarded Fire cards exhaust the recipe.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::Crispin) && !after.supporter_used,
         "A known dead Crispin route must preserve the Supporter turn.");
}

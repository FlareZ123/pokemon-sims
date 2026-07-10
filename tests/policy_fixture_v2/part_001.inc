  expect(contains(after.hand, Card::RegidragoVstar), "Gladion should retrieve prized VSTAR.");
  expect(contains(after.prizes, Card::Gladion), "Gladion must be shuffled into the remaining Prize cards.");
  expect(!contains(after.discard, Card::Gladion), "Gladion must not enter discard after its Prize exchange.");
}

void test_gladion_is_not_burned_when_vstar_is_already_in_hand() {
  State s = state(2, Card::RegidragoV, 0, 1, 0);
  s.hand = {Card::Gladion, Card::RegidragoVstar};
  s.prizes = {Card::RegidragoVstar};
  Fixture f(scenario("gladion-held"), s);
  expect(!EngineTestAccess::play_gladion(f.engine), "Gladion should be held when VSTAR is already in hand.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Gladion), "Held Gladion must remain in hand.");
}

void test_steven_fetches_burnet_before_turn_two_item_lock() {
  State s = state(1);
  s.hand = {Card::StevensResolve};
  // Crispin's printed search needs Basic Energy of different types: https://api.pokemontcg.io/v2/cards/sv7-133
  // Burnet must have a Dragon payload it can discard from deck: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  s.deck = {Card::RegidragoVstar, Card::Crispin, Card::ProfessorBurnet, Card::BrilliantBlender, Card::MegaDragonite, Card::Grass, Card::Fire};
  Fixture f(scenario("steven-lock", DciProfile::StrictJit, LockMode::TurnTwoItem), s);
  expect(EngineTestAccess::play_steven(f.engine), "Steven should bridge three missing axes on going-second turn one.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::RegidragoVstar) && contains(after.hand, Card::Crispin) && contains(after.hand, Card::ProfessorBurnet),
         "Steven should choose VSTAR, Crispin, and Burnet under turn-two Item lock.");
  expect(!contains(after.hand, Card::BrilliantBlender), "Steven must not choose Blender for an Item-locked next turn.");
  expect(after.turn_ended, "Steven's Resolve must end the turn.");
}

void test_steven_uses_legal_fallback_when_preferred_card_is_prized() {
  State s = state(1);
  s.hand = {Card::StevensResolve};
  s.prizes = {Card::RegidragoVstar};
  // Crispin is asserted as a live fallback only when this fixture contains its searchable Grass and Fire pair: https://api.pokemontcg.io/v2/cards/sv7-133
  // Burnet is a live fallback only with a payload it can search and discard: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  s.deck = {Card::Crispin, Card::ProfessorBurnet, Card::EarthenVessel, Card::MegaDragonite, Card::Grass, Card::Fire};
  Fixture f(scenario("steven-fallback", DciProfile::StrictJit, LockMode::TurnTwoItem), s);
  expect(EngineTestAccess::play_steven(f.engine), "Steven should still be playable with a prized preferred VSTAR.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(contains(after.hand, Card::Crispin) && contains(after.hand, Card::ProfessorBurnet) && contains(after.hand, Card::EarthenVessel),
         "Steven should fill a failed search slot with a legal useful fallback.");
}

void test_fss_finds_steven_on_turn_one_when_three_axes_are_missing() {
  State s = state(1);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::StevensResolve};
  Fixture f(scenario("fss-steven"), s);
  expect(EngineTestAccess::use_fss(f.engine), "Forest Seal Stone should use Star Alchemy for Steven's Resolve.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::StevensResolve), "Star Alchemy should find Steven's Resolve.");
  expect(EngineTestAccess::state(f.engine).vstar_power_used, "Star Alchemy must consume the one VSTAR Power.");
}

void test_fss_uses_a_fallback_when_vstar_is_not_in_deck() {
  State s = state(2, Card::RegidragoV, 0, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.prizes = {Card::RegidragoVstar};
  // Professor Burnet can search and discard this Mega Dragonite payload: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  s.deck = {Card::Crispin, Card::ProfessorBurnet, Card::MegaDragonite};
  Fixture f(scenario("fss-fallback"), s);
  expect(EngineTestAccess::use_fss(f.engine), "Forest Seal Stone should be usable when VSTAR is prized.");
  // Crispin has no legal Energy target after Star Alchemy inspects this deck; Burnet remains the live payload outlet: https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::ProfessorBurnet), "Star Alchemy should take the remaining live payload fallback instead of dead Crispin.");
}

// Historical fixture identifier retained because the runner list is split across include files.
void test_fss_cannot_be_used_from_a_vstar_holder() {
  State s = state(2, Card::RegidragoVstar, 0, 2, 0);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::Fire};
  Fixture f(scenario("fss-vstar"), s);
  // Pokémon V includes Pokémon VSTAR, so the evolved holder retains Star Alchemy:
  // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  expect(EngineTestAccess::use_fss(f.engine),
         "Forest Seal Stone must grant Star Alchemy to the evolved Pokémon VSTAR holder.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::Fire),
         "Star Alchemy should search the missing Fire Energy from the VSTAR holder.");
}

void test_existing_fss_is_usable_under_item_lock() {
  State s = state(2, Card::RegidragoV, 0, 1, 0);
  s.active->tool = Tool::ForestSealStone;
  s.deck = {Card::RegidragoVstar};
  Fixture f(scenario("fss-item-lock", DciProfile::StrictJit, LockMode::FullItem), s);
  expect(EngineTestAccess::use_fss(f.engine), "Existing Forest Seal Stone should be usable without playing an Item from hand.");
  expect(contains(EngineTestAccess::state(f.engine).hand, Card::RegidragoVstar), "Star Alchemy should still search VSTAR under Item lock.");
}

void test_crispin_attaches_only_with_two_different_energy_types() {
  State s = state(2, Card::RegidragoV, 0, 1, 0);
  s.hand = {Card::Crispin};
  s.deck = {Card::Grass, Card::Fire};
  Fixture f(scenario("crispin-two-types"), s);
  expect(EngineTestAccess::play_crispin(f.engine), "Crispin should play with Grass and Fire searchable.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(after.active->grass == 2 && contains(after.hand, Card::Fire), "Crispin should attach Grass and put Fire in hand for the GGF target.");
}

void test_crispin_single_type_only_puts_energy_in_hand() {
  State s = state(2, Card::RegidragoV, 0, 1, 0);
  s.hand = {Card::Crispin};
  s.deck = {Card::Fire};
  Fixture f(scenario("crispin-single-type"), s);
  expect(EngineTestAccess::play_crispin(f.engine), "Crispin may still resolve with only one Basic Energy type available.");
  const State& after = EngineTestAccess::state(f.engine);
  expect(after.active->grass == 1 && after.active->fire == 0 && contains(after.hand, Card::Fire),
         "With one type only, Crispin must attach none and put the found Energy into hand.");
}

void test_earthen_vessel_can_take_two_grass() {
  State s = state(2, Card::RegidragoV, 0, 0, 1);
  s.hand = {Card::EarthenVessel, Card::Dipplin};
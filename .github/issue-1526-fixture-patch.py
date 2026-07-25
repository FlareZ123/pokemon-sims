from pathlib import Path

path = Path("tests/issue_1526_tate_dark_asset_tests.cpp")
source = path.read_text(encoding="utf-8")
start = source.index("sim::State deterministic_vstar_state")
end = source.index("}  // namespace", start)
replacement = r'''sim::State deterministic_treasure_state(const bool dark_asset_resolved) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::TateLiza, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin};
  state.manual_energy_used = true;
  state.dark_asset_used = dark_asset_resolved;
  if (dark_asset_resolved) {
    state.bench = {sim::Pokemon{sim::Card::CrobatV, 2}};
  } else {
    state.hand.push_back(sim::Card::CrobatV);
  }
  return state;
}

void test_public_held_treasure_completion_with_unresolved_crobat_remains_live() {
  const sim::Scenario scenario{"issue-1526-public", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{152601};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, deterministic_treasure_state(false));

  // Mysterious Treasure can deterministically discard the held Dragon payload,
  // search Regidrago VSTAR, and evolve the prior-turn Active. An unused Crobat V
  // cannot make that public route probabilistic or force Tate & Liza to be spent:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Core Item, discard, search, evolution, Ability, and Supporter procedure:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
  expect(sim::EngineTestAccess::tate_route_completes(engine),
         "Public held Treasure completion was suppressed by an unused Crobat V");
}

void test_completion_after_real_dark_asset_resolution_remains_live() {
  const sim::Scenario scenario{"issue-1526-resolved", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{152602};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, deterministic_treasure_state(true));

  // Once Dark Asset has legally resolved in the real public state, the held Treasure
  // and payload remain a deterministic VSTAR completion:
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Ability, draw, Item, discard, search, Supporter, and evolution procedure:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1526
  expect(sim::EngineTestAccess::tate_route_completes(engine),
         "A legally resolved Dark Asset state lost its held Treasure completion");
}

'''
source = source[:start] + replacement + source[end:]
source = source.replace(
    "  test_public_held_vstar_completion_with_unresolved_crobat_remains_live();",
    "  test_public_held_treasure_completion_with_unresolved_crobat_remains_live();",
)
path.write_text(source, encoding="utf-8")

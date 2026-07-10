#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool attach_live_fss(Engine& engine) { return engine.attach_live_fss(); }
  static bool attach_powerglass(Engine& engine) { return engine.attach_powerglass(); }
  static bool resolve_powerglass_end_turn(Engine& engine) { return engine.resolve_powerglass_end_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_powerglass_energy_counts_on_the_following_turn() {
  const sim::Scenario scenario{"powerglass-delayed-energy", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{201};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Powerglass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Powerglass attaches a Basic Energy from discard at the end of the turn after
  // the attack timing point: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "Powerglass should attach to the Active Regidrago VSTAR with an open Tool slot");
  expect(sim::EngineTestAccess::state(engine).active->tool == sim::Tool::Powerglass,
         "The Active Regidrago VSTAR should hold Powerglass");

  sim::EngineTestAccess::record_ready(engine);
  expect(sim::EngineTestAccess::outcome(engine).first_ready_turn == 0,
         "Powerglass must not make the already-passed turn-2 attack ready");

  expect(sim::EngineTestAccess::resolve_powerglass_end_turn(engine),
         "Powerglass should attach the needed Fire Energy at end of turn");
  expect(sim::EngineTestAccess::state(engine).active->fire == 1,
         "The recovered Fire Energy should be attached to the Active holder");
  expect(!contains(sim::EngineTestAccess::state(engine).discard, sim::Card::Fire),
         "The attached Fire Energy should leave discard");
  expect(sim::EngineTestAccess::outcome(engine).first_ready_turn == 0,
         "End-of-turn recovery must not retroactively count as turn-2 readiness");

  sim::EngineTestAccess::state(engine).turn = 3;
  sim::EngineTestAccess::record_ready(engine);
  expect(sim::EngineTestAccess::outcome(engine).first_ready_turn == 3,
         "The Powerglass Energy may contribute to readiness on turn 3");
}

void test_powerglass_is_attachable_through_item_lock() {
  const sim::Scenario scenario{"powerglass-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{202};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Powerglass};
  state.discard = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Pokémon Tools are a category separate from Item cards, so Item lock does not
  // prohibit this attachment: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "Powerglass should remain attachable through Item lock");
}

void test_arven_falls_back_to_powerglass_after_fss_search_miss() {
  const sim::Scenario scenario{"arven-powerglass-search-fallback", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{304};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::Powerglass, sim::Card::Fire};
  state.prizes = {sim::Card::ForestSealStone};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven may select a Pokémon Tool. After legal deck inspection proves the
  // higher-priority Forest Seal Stone absent, Powerglass remains a legal live Tool
  // target and is attachable through Item lock:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  expect(sim::EngineTestAccess::play_arven(engine),
         "Arven should be played for the plausible Forest Seal Stone or Powerglass Tool route");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Powerglass),
         "Arven should take Powerglass after the search proves Forest Seal Stone absent");
  expect(!contains(sim::EngineTestAccess::state(engine).deck, sim::Card::Powerglass),
         "The selected Powerglass should leave the deck");
  expect(contains(sim::EngineTestAccess::state(engine).prizes, sim::Card::ForestSealStone),
         "The face-down Forest Seal Stone should remain in Prizes");
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "The searched Powerglass should attach to the Active Regidrago VSTAR");
}

void test_powerglass_does_not_resolve_from_the_bench() {
  const sim::Scenario scenario{"powerglass-benched-holder", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{203};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::Powerglass}};
  state.discard = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Powerglass requires the attached Pokémon to be in the Active Spot:
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  expect(!sim::EngineTestAccess::resolve_powerglass_end_turn(engine),
         "A benched Powerglass holder must not attach Energy");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::Fire),
         "The Fire Energy should remain in discard while the holder is Benched");
}

void test_spent_vstar_power_preserves_tool_slot_for_powerglass() {
  const sim::Scenario scenario{"spent-vstar-power-tool-slot", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{204};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone, sim::Card::Powerglass};
  state.discard = {sim::Card::Fire};
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Forest Seal Stone grants a VSTAR Power, but a player may use only one VSTAR
  // Power per game. Powerglass is the live Tool when that resource is spent:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  expect(!sim::EngineTestAccess::attach_live_fss(engine),
         "Forest Seal Stone should be held after the VSTAR Power is spent");
  expect(sim::EngineTestAccess::state(engine).active->tool == sim::Tool::None,
         "The spent Stone must leave the one Tool slot open");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::ForestSealStone),
         "The dead Forest Seal Stone should remain in hand");
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "Powerglass should use the preserved Tool slot for the live Energy route");
  expect(sim::EngineTestAccess::state(engine).active->tool == sim::Tool::Powerglass,
         "The Active Regidrago VSTAR should hold the live Powerglass");
}

void test_fss_uses_bench_when_powerglass_needs_active_tool_slot() {
  const sim::Scenario scenario{"fss-bench-powerglass-active", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{205};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::ForestSealStone, sim::Card::Powerglass};
  state.discard = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Forest Seal Stone has no Active requirement, Powerglass checks its Active holder,
  // and each Pokémon has one Tool slot. The legal combined line puts the Stone on the
  // Benched Pokémon V and reserves the Active slot for Powerglass:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect(sim::EngineTestAccess::attach_live_fss(engine),
         "Forest Seal Stone should attach to the eligible Benched Pokémon V");
  expect(sim::EngineTestAccess::state(engine).active->tool == sim::Tool::None,
         "The Active Tool slot should remain open for Powerglass");
  expect(sim::EngineTestAccess::state(engine).bench.front().tool == sim::Tool::ForestSealStone,
         "The Benched Pokémon V should hold Forest Seal Stone");
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "Powerglass should attach to the reserved Active Tool slot");
  expect(sim::EngineTestAccess::state(engine).active->tool == sim::Tool::Powerglass,
         "The Active Regidrago VSTAR should hold Powerglass");
}

void test_powerglass_uses_its_active_holder_instead_of_a_benched_regidrago() {
  const sim::Scenario scenario{"powerglass-active-holder-energy", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{221};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::Powerglass};
  state.discard = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Powerglass can attach only to the Pokémon holding it in the Active Spot:
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  expect(sim::EngineTestAccess::attach_powerglass(engine),
         "The complete Benched VSTAR must not hide the Active holder's missing Fire Energy");
  expect(sim::EngineTestAccess::resolve_powerglass_end_turn(engine),
         "Powerglass should resolve from the Active holder's own Energy deficit");
  expect(sim::EngineTestAccess::state(engine).active->fire == 1,
         "The Active Powerglass holder should receive the missing Fire Energy");
  expect(sim::EngineTestAccess::state(engine).bench.front().fire == 1,
         "The already complete Benched Regidrago VSTAR must remain unchanged");
}

}  // namespace

int main() {
  try {
    test_powerglass_energy_counts_on_the_following_turn();
    test_powerglass_is_attachable_through_item_lock();
    test_arven_falls_back_to_powerglass_after_fss_search_miss();
    test_powerglass_does_not_resolve_from_the_bench();
    test_spent_vstar_power_preserves_tool_slot_for_powerglass();
    test_fss_uses_bench_when_powerglass_needs_active_tool_slot();
    test_powerglass_uses_its_active_holder_instead_of_a_benched_regidrago();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
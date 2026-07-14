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
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_roseanne_restores_missing_fire_for_same_turn_vessel_attachment() {
  const sim::Scenario scenario{"roseanne-energy-vessel", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{305};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::EarthenVessel, sim::Card::Dipplin};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Roseanne's Backup may shuffle an Energy from discard into the deck. Earthen
  // Vessel can then search that Basic Energy, and the unused once-per-turn manual
  // attachment can put it on the Active Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar && after.active->fire == 1,
         "The restored Fire Energy should be searched and manually attached in the same turn");
  expect(after.supporter_used && after.manual_energy_used,
         "The Roseanne Supporter play and manual Energy attachment should both be consumed");
  expect(contains(after.discard, sim::Card::RoseannesBackup) &&
             contains(after.discard, sim::Card::EarthenVessel) && contains(after.discard, sim::Card::Dipplin),
         "Roseanne, Earthen Vessel, and the Vessel cost should be in discard");
  expect(!contains(after.deck, sim::Card::Fire) && !contains(after.hand, sim::Card::Fire),
         "The restored Fire Energy should finish attached rather than remain in deck or hand");
}

void test_roseanne_restores_vstar_and_energy_in_one_supporter_play() {
  const sim::Scenario scenario{"roseanne-vstar-energy-multimode", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{321};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::EvolutionIncense,
                sim::Card::EarthenVessel, sim::Card::Dipplin};
  state.deck = {sim::Card::Fire};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  // Forest Seal Stone or another VSTAR Power may already have been used. A player
  // may use only one VSTAR Power during a game, so Legacy Star cannot rescue this route:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Roseanne's Backup says "Choose 1 or more," so the Pokémon and Energy modes
  // may restore Regidrago VSTAR and Grass together:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // Evolution Incense then finds the restored Evolution Pokémon, Earthen Vessel
  // finds the restored Basic Energy, and the unused manual attachment completes GGF:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Play_Basic_Energy_Cards
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Roseanne's combined Pokémon and Energy modes should complete the Active Regidrago VSTAR");
  expect(after.supporter_used && after.manual_energy_used,
         "The combined Roseanne route should consume the Supporter and manual attachment");
  expect(contains(after.discard, sim::Card::RoseannesBackup) &&
             contains(after.discard, sim::Card::EvolutionIncense) &&
             contains(after.discard, sim::Card::EarthenVessel) &&
             contains(after.discard, sim::Card::Dipplin),
         "The full same-turn search chain should resolve into discard");
  expect(!contains(after.deck, sim::Card::RegidragoVstar) &&
             !contains(after.deck, sim::Card::Grass) &&
             !contains(after.hand, sim::Card::RegidragoVstar) &&
             !contains(after.hand, sim::Card::Grass),
         "The restored VSTAR and Grass should finish evolved and attached");
}

void test_roseanne_does_not_reuse_incense_as_vessel_cost() {
  const sim::Scenario scenario{"roseanne-no-shared-search-cost", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{322};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::EvolutionIncense, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Fire};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  // Keep Legacy Star unavailable so the test isolates the two future Item costs:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Evolution Incense leaves hand before Earthen Vessel must discard another card.
  // The same Incense cannot also pay Vessel's later cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv4-163
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 1 && after.active->fire == 1,
         "The VSTAR-only recovery may resolve, while the unpayable Energy continuation must remain incomplete");
  expect(after.supporter_used && !after.manual_energy_used,
         "The unpayable Vessel line must not consume the manual attachment");
  expect(contains(after.discard, sim::Card::Grass),
         "Grass must remain in discard when the combined route lacks a distinct Vessel cost");
  expect(contains(after.hand, sim::Card::EarthenVessel),
         "Earthen Vessel must remain held when no other card can pay its cost");
}

void test_roseanne_energy_route_discards_an_extra_dragon_after_payload_is_ready() {
  const sim::Scenario scenario{"roseanne-extra-dragon-vessel-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{454};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::EarthenVessel, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  // Keep Legacy Star unavailable so this test isolates Roseanne and Earthen Vessel:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Mega Dragonite already establishes the strict-JIT payload. Earthen Vessel may
  // discard the additional Dragon as its required other card, which also adds that
  // attack to Apex Dragon's legal discard-pile choices:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->fire == 1,
         "The restored Fire Energy should complete the Active Regidrago VSTAR");
  expect(after.supporter_used && after.manual_energy_used,
         "The Roseanne route and manual attachment should resolve");
  expect(contains(after.discard, sim::Card::Dragapult),
         "The extra Dragon should legally pay Earthen Vessel's discard cost");
  expect(contains(after.discard, sim::Card::EarthenVessel) &&
             contains(after.discard, sim::Card::RoseannesBackup),
         "The resolved Trainer cards should be in discard");
}

void test_roseanne_energy_route_still_requires_a_distinct_vessel_cost() {
  const sim::Scenario scenario{"roseanne-energy-no-vessel-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{455};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  // Keep Legacy Star unavailable so it cannot recover the missing Energy independently:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Earthen Vessel requires one other card from hand. Payload readiness does not
  // waive that cost:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->fire == 0,
         "The Energy route must remain incomplete without another hand card");
  expect(!after.supporter_used && !after.manual_energy_used,
         "Roseanne and the manual attachment must be preserved when Vessel is unpayable");
  expect(contains(after.hand, sim::Card::RoseannesBackup) &&
             contains(after.hand, sim::Card::EarthenVessel),
         "Both Trainers must remain in hand when the route cannot pay its cost");
}

}  // namespace

int main() {
  try {
    test_roseanne_restores_missing_fire_for_same_turn_vessel_attachment();
    test_roseanne_restores_vstar_and_energy_in_one_supporter_play();
    test_roseanne_does_not_reuse_incense_as_vessel_cost();
    test_roseanne_energy_route_discards_an_extra_dragon_after_payload_is_ready();
    test_roseanne_energy_route_still_requires_a_distinct_vessel_cost();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

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
  static bool play_klara(Engine& engine) { return engine.play_klara_recovery(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void play_items(Engine& engine) { engine.play_items_until_stable(true); }
};

}  // namespace sim

namespace {

int count_of(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return count_of(cards, card) > 0;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_registered_shell_permanently_swaps_roseanne_for_klara() {
  const sim::DeckRecipe shell = sim::baseline_recipe();
  const sim::DeckRecipe pineco = sim::pineco_recipe();
  const auto copies = [](const sim::DeckRecipe& recipe, const sim::Card card) {
    int result = 0;
    for (const auto& [candidate, count] : recipe) {
      if (candidate == card) result += count;
    }
    return result;
  };

  expect(copies(shell, sim::Card::Klara) == 1,
         "The registered shell must contain one Klara");
  expect(copies(shell, sim::Card::RoseannesBackup) == 0,
         "The registered shell must no longer contain Roseanne's Backup");
  expect(copies(pineco, sim::Card::Klara) == 0 &&
             copies(pineco, sim::Card::RoseannesBackup) == 0,
         "The Pineco recipe contains neither card and must remain unchanged");
  expect(sim::is_supporter(sim::Card::Klara) && sim::name(sim::Card::Klara) == "Klara",
         "Klara must be a named Supporter");
}

void test_klara_recovers_vstar_energy_and_redundant_older_dragon() {
  const sim::Scenario scenario{"klara-vstar-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1773};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);

  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Klara};
  state.deck = {sim::Card::Fire};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::Grass,
                   sim::Card::Dragapult, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  // Klara chooses both printed modes. It puts the missing VSTAR and Grass directly
  // into hand, while its second Pokémon selection may lift an older Dragon to deny a
  // later discard-control window. The current-turn Dragon remains in discard so the
  // strict-JIT payload is not broken:
  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, evolution, and Energy procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/1773
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Klara should recover the VSTAR and missing Grass for same-turn completion");
  expect(after.supporter_used && after.manual_energy_used,
         "The Klara Supporter and manual attachment must both be consumed");
  expect(contains(after.discard, sim::Card::Klara) &&
             contains(after.discard, sim::Card::MegaDragonite) &&
             contains(after.discarded_this_turn, sim::Card::MegaDragonite),
         "Klara must preserve the current-turn strict-JIT Dragon in discard");
  expect(contains(after.hand, sim::Card::Dragapult),
         "Klara should use its second Pokémon selection for the older Dragon");
}

void test_klara_recovers_old_payload_for_a_current_turn_item_discard() {
  const sim::Scenario scenario{"klara-payload-replay", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1774};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);

  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Klara, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Dipplin};
  state.discard = {sim::Card::MegaDragonite};
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_klara(engine),
         "Klara should recover an older Dragon when a payable Item can discard it");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite),
         "The recovered Dragon should enter hand, not the deck");
  sim::EngineTestAccess::play_items(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MegaDragonite) &&
             contains(after.discarded_this_turn, sim::Card::MegaDragonite),
         "The recovered Dragon should be discarded again this turn for strict-JIT");
  expect(contains(after.hand, sim::Card::Dipplin),
         "Mysterious Treasure should resolve its legal search after paying the Dragon cost");
}

void test_klara_obeys_two_pokemon_and_two_energy_limits() {
  const sim::Scenario scenario{"klara-up-to-two", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1775};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);

  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Klara};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                   sim::Card::GoodraVstar, sim::Card::Grass,
                   sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_klara(engine),
         "Klara should choose both legal recovery modes");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_of(after.hand, sim::Card::RegidragoVstar) == 1,
         "Klara should recover the advancing VSTAR");
  expect(count_of(after.hand, sim::Card::Dragapult) +
             count_of(after.hand, sim::Card::GoodraVstar) == 1,
         "Klara must recover at most one additional Pokémon after the VSTAR");
  expect(count_of(after.hand, sim::Card::Grass) == 2 &&
             count_of(after.hand, sim::Card::Fire) == 0,
         "Klara must recover at most two Basic Energy cards");
}

void test_klara_does_not_remove_the_only_ready_payload() {
  const sim::Scenario scenario{"klara-preserve-ready-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1776};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);

  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Klara};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_klara(engine),
         "The missing Fire Energy should make Klara worth playing");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Fire),
         "Klara should recover the missing Fire Energy");
  expect(!contains(after.hand, sim::Card::MegaDragonite) &&
             contains(after.discard, sim::Card::MegaDragonite) &&
             contains(after.discarded_this_turn, sim::Card::MegaDragonite),
         "Klara must not recover the sole current-turn payload");
}

}  // namespace

int main() {
  try {
    test_registered_shell_permanently_swaps_roseanne_for_klara();
    test_klara_recovers_vstar_energy_and_redundant_older_dragon();
    test_klara_recovers_old_payload_for_a_current_turn_item_discard();
    test_klara_obeys_two_pokemon_and_two_energy_limits();
    test_klara_does_not_remove_the_only_ready_payload();
  } catch (const std::exception& error) {
    std::cerr << "klara_recovery_tests failed: " << error.what() << '\n';
    return 1;
  }
  return 0;
}

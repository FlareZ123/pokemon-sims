#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_oricorio_if_useful(Engine& engine) { return engine.bench_oricorio_if_useful(); }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool play_turo_oricorio_energy_route(Engine& engine) {
    return engine.play_turo_oricorio_energy_route();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_off_type_fire_does_not_hide_missing_grass_connector() {
  // Regidrago V needs two Grass and one Fire Energy: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Vital Dance can search the remaining Basic Grass Energy: https://api.pokemontcg.io/v2/cards/sm2-55
  sim::Scenario scenario{"oricorio-type-sensitive-energy", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Oricorio, sim::Card::Fire};
  state.deck = {sim::Card::Grass};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{107};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::bench_oricorio_if_useful(engine),
         "An off-type Fire Energy must not suppress Vital Dance for a missing Grass Energy.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Grass),
         "Vital Dance should put the missing Grass Energy into hand.");
}

void test_turo_replays_oricorio_for_the_final_energy() {
  // Professor Turo returns a Pokémon in play to hand, Oricorio triggers Vital Dance
  // when replayed from hand to the Bench, and the turn still permits one manual Energy
  // attachment: https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"turo-oricorio-final-energy", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::ProfessorTuro};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{108};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  sim::EngineTestAccess::choose_supporter(engine);
  expect(sim::EngineTestAccess::state(engine).supporter_used,
         "Professor Turo should use the available Supporter play for the exact Energy route.");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::ProfessorTuro),
         "Professor Turo should be in discard after resolving.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Fire),
         "Replayed Oricorio should search the missing Fire Energy with Vital Dance.");
  expect(std::any_of(sim::EngineTestAccess::state(engine).bench.begin(),
                     sim::EngineTestAccess::state(engine).bench.end(),
                     [](const sim::Pokemon& pokemon) { return pokemon.card == sim::Card::Oricorio; }),
         "Oricorio should be replayed to the Bench.");
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The unused manual attachment should attach the searched Fire Energy.");
  expect(sim::EngineTestAccess::state(engine).active->fire == 1,
         "The Active Regidrago VSTAR should reach GGF.");
}

void test_turo_discards_oricorio_attachments_before_replay() {
  // Professor Turo discards every card attached to the returned Pokémon:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // The replayed Oricorio is a new Benched object and Vital Dance resolves from hand:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  sim::Scenario scenario{"turo-oricorio-attached-cleanup", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1, 1, 1, sim::Tool::Powerglass}};
  state.hand = {sim::Card::ProfessorTuro};
  state.deck = {sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{109};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_turo_oricorio_energy_route(engine),
         "The deterministic Turo-to-Oricorio route should resolve.");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::Grass),
         "Attached Grass Energy should be discarded.");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::Fire),
         "Attached Fire Energy should be discarded.");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::Powerglass),
         "Attached Powerglass should be discarded.");
  const auto replayed = std::find_if(sim::EngineTestAccess::state(engine).bench.begin(),
                                     sim::EngineTestAccess::state(engine).bench.end(),
                                     [](const sim::Pokemon& pokemon) {
                                       return pokemon.card == sim::Card::Oricorio;
                                     });
  expect(replayed != sim::EngineTestAccess::state(engine).bench.end(),
         "Oricorio should return to the Bench after Turo resolves.");
  expect(replayed->grass == 0 && replayed->fire == 0 && replayed->tool == sim::Tool::None,
         "The replayed Oricorio must have no inherited attachments.");
}

}  // namespace

int main() {
  try {
    test_off_type_fire_does_not_hide_missing_grass_connector();
    test_turo_replays_oricorio_for_the_final_energy();
    test_turo_discards_oricorio_attachments_before_replay();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

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

void test_turo_completes_the_active_vstar_even_when_a_benched_vstar_scores_higher() {
  // Professor Turo returns Oricorio, replaying it triggers Vital Dance, and the player
  // may use the turn's one manual Energy attachment on the Active Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"turo-oricorio-active-specific-energy", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::ProfessorTuro};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{108};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used,
         "Professor Turo should use the Supporter play for the Active-specific Energy route.");
  expect(after.manual_energy_used,
         "The route should consume the turn's manual Energy attachment.");
  expect(after.active && after.active->fire == 1 && after.active->grass == 2,
         "The Active Regidrago VSTAR should reach GGF even when a Benched VSTAR scores higher.");
  expect(contains(after.discard, sim::Card::ProfessorTuro),
         "Professor Turo should be in discard after resolving.");
  expect(std::any_of(after.bench.begin(), after.bench.end(), [](const sim::Pokemon& pokemon) {
           return pokemon.card == sim::Card::Oricorio;
         }),
         "Oricorio should be replayed to the Bench.");
}

void test_turo_discards_oricorio_attachments_before_replay() {
  // Professor Turo discards every card attached to the returned Pokémon:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // Replaying Oricorio is a new Bench placement that triggers Vital Dance:
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
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{109};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_turo_oricorio_energy_route(engine),
         "The deterministic Turo-to-Oricorio route should resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::Grass) == 1,
         "Attached Grass Energy should be discarded exactly once.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::Fire) == 1,
         "Attached Fire Energy should be discarded exactly once.");
  expect(contains(after.discard, sim::Card::Powerglass),
         "Attached Powerglass should be discarded.");
  const auto replayed = std::find_if(after.bench.begin(), after.bench.end(), [](const sim::Pokemon& pokemon) {
    return pokemon.card == sim::Card::Oricorio;
  });
  expect(replayed != after.bench.end(), "Oricorio should return to the Bench after Turo resolves.");
  expect(replayed->grass == 0 && replayed->fire == 0 && replayed->tool == sim::Tool::None,
         "The replayed Oricorio must have no inherited attachments.");
  expect(after.active && after.active->fire == 1 && after.manual_energy_used,
         "The searched Fire Energy should be manually attached to the Active VSTAR.");
}

void test_turo_defers_to_crispin_for_the_same_exact_energy() {
  // Crispin is the direct Energy Supporter and should remain preferred when it can
  // obtain the same missing Basic Energy: https://api.pokemontcg.io/v2/cards/sv7-133
  sim::Scenario scenario{"turo-defers-to-crispin", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::Crispin};
  state.deck = {sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{110};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_turo_oricorio_energy_route(engine),
         "Professor Turo should defer while Crispin can obtain the exact missing Energy.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::ProfessorTuro),
         "Deferring to Crispin must preserve Professor Turo in hand.");
}

}  // namespace

int main() {
  try {
    test_off_type_fire_does_not_hide_missing_grass_connector();
    test_turo_completes_the_active_vstar_even_when_a_benched_vstar_scores_higher();
    test_turo_discards_oricorio_attachments_before_replay();
    test_turo_defers_to_crispin_for_the_same_exact_energy();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}

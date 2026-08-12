#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_finish_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.issue_1797_finish_turn_ = engine.state_.turn;
  }

  static bool finish(Engine& engine) {
    return engine.issue_3316_1797_finish_continuation();
  }

  static bool manual_energy_used(const Engine& engine) {
    return engine.state_.manual_energy_used;
  }

  static int active_fire(const Engine& engine) {
    return engine.state_.active ? engine.state_.active->fire : -1;
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State finish_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  const sim::Scenario scenario{"issue-3334", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  return sim::Engine(scenario, sim::baseline_recipe(), rng);
}

void test_fire_only_search_spends_manual_attachment() {
  // Crispin may search one Basic Energy because its search says "up to 2", but
  // the searched card is put into hand and there is no second searched Energy
  // available as "the other Energy" for Crispin to attach.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // "Up to" and partial-resolution rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3334
  std::mt19937_64 rng(3334);
  sim::Engine engine = make_engine(rng);
  sim::State state = finish_state();
  state.deck = {sim::Card::Fire, sim::Card::Fire};
  sim::EngineTestAccess::set_finish_state(engine, std::move(state));

  expect(sim::EngineTestAccess::finish(engine),
         "Fire-only Crispin continuation did not finish through the manual attachment");
  expect(sim::EngineTestAccess::manual_energy_used(engine),
         "Fire-only Crispin search incorrectly received a free Supporter attachment");
  expect(sim::EngineTestAccess::active_fire(engine) == 1,
         "Fire-only continuation did not end with exactly one Fire attached");
}

void test_two_type_search_keeps_crispin_attachment() {
  // With two different searched Basic Energy types, Crispin can put one into hand
  // and attach the other, leaving the normal manual attachment unused.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3334
  std::mt19937_64 rng(3335);
  sim::Engine engine = make_engine(rng);
  sim::State state = finish_state();
  state.deck = {sim::Card::Fire, sim::Card::Fire, sim::Card::Grass};
  sim::EngineTestAccess::set_finish_state(engine, std::move(state));

  expect(sim::EngineTestAccess::finish(engine),
         "two-type Crispin continuation stopped resolving");
  expect(!sim::EngineTestAccess::manual_energy_used(engine),
         "two-type Crispin search consumed the manual attachment");
  expect(sim::EngineTestAccess::active_fire(engine) == 1,
         "two-type continuation did not attach exactly one Fire");
}

void test_grass_only_search_uses_held_fire_manually() {
  // A one-type Grass search likewise supplies no second searched Energy to attach;
  // a Fire already in hand must use the normal once-per-turn attachment instead.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Official Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3334
  std::mt19937_64 rng(3336);
  sim::Engine engine = make_engine(rng);
  sim::State state = finish_state();
  state.hand.push_back(sim::Card::Fire);
  state.deck = {sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_finish_state(engine, std::move(state));

  expect(sim::EngineTestAccess::finish(engine),
         "Grass-only Crispin continuation did not finish with held Fire");
  expect(sim::EngineTestAccess::manual_energy_used(engine),
         "Grass-only Crispin search incorrectly preserved the manual attachment");
  expect(sim::EngineTestAccess::active_fire(engine) == 1,
         "Grass-only continuation did not manually attach exactly one Fire");
}

}  // namespace

int main() {
  test_fire_only_search_spends_manual_attachment();
  test_two_type_search_keeps_crispin_attachment();
  test_grass_only_search_uses_held_fire_manually();
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool paid_blender_route(Engine& engine) {
    return engine.complete_paid_one_cost_basic_retreat_with_held_blender();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const sim::Card active) {
  sim::State state;
  state.turn = 3;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{active, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1}};
  state.hand = {sim::Card::Grass, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng, sim::State state) {
  const sim::Scenario scenario{"issue-2295-semantic-retreat-gate",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, nullptr);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_one_cost_basic_is_accepted_semantically() {
  std::mt19937_64 rng{229501};
  sim::Engine engine = make_engine(rng, route_state(sim::Card::MawileGX));

  // Mawile-GX is a Basic with printed Retreat Cost 1, so it exercises the
  // semantic gate beyond the two historical witnesses while keeping the exact
  // same legal attach -> retreat -> Brilliant Blender continuation:
  // Mawile-GX: https://api.pokemontcg.io/v2/cards/sm11-141
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Retreat and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // State-driven route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
  expect(sim::is_basic(sim::Card::MawileGX), "Mawile-GX must be modeled as Basic.");
  expect(sim::retreat_cost(sim::Card::MawileGX) == 1,
         "Mawile-GX must retain its printed one-Energy Retreat Cost.");
  expect(sim::EngineTestAccess::paid_blender_route(engine),
         "A legal one-cost Basic Active was rejected by the semantic route.");
}

void test_higher_cost_basic_is_rejected() {
  std::mt19937_64 rng{229502};
  sim::Engine engine = make_engine(rng, route_state(sim::Card::RegidragoV));

  // Regidrago V is also Basic, but its printed Retreat Cost is 3. One held Basic
  // Energy cannot pay that cost, so the one-cost route must reject it before
  // spending the manual attachment or Retreat action:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Official Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
  expect(sim::is_basic(sim::Card::RegidragoV), "Regidrago V must be modeled as Basic.");
  expect(sim::retreat_cost(sim::Card::RegidragoV) == 3,
         "Regidrago V must retain its printed three-Energy Retreat Cost.");
  expect(!sim::EngineTestAccess::paid_blender_route(engine),
         "A Basic with Retreat Cost greater than 1 entered the one-cost route.");
  expect(!engine.state().manual_energy_used && !engine.state().retreat_used,
         "The rejected higher-cost Basic route spent a once-per-turn action.");
}

}  // namespace

int main() {
  test_one_cost_basic_is_accepted_semantically();
  test_higher_cost_basic_is_rejected();
  return 0;
}

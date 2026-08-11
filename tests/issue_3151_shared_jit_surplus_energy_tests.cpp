#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <optional>
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
  static std::optional<Card> treasure_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true, Card::MysteriousTreasure);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::BrilliantBlender};
  state.prizes = {sim::Card::Arven, sim::Card::Gladion,
                  sim::Card::ForestSealStone, sim::Card::Lusamine,
                  sim::Card::QuickBall, sim::Card::LatiasEx};
  return state;
}

std::optional<sim::Card> cost_for(const sim::DciProfile dci) {
  const sim::Scenario scenario{"issue-3151", dci, sim::LockMode::None,
                               false, 3};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3151);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());
  return sim::EngineTestAccess::treasure_cost(engine);
}

void test_both_same_turn_jit_profiles_spend_only_surplus_grass() {
  // The route reserves exactly one Grass and one Fire for the remaining manual
  // attachments. The second Grass is the same route-proven DCI cost under both
  // profiles that require the payload on the actual ready turn:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Shared JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3151
  expect(cost_for(sim::DciProfile::StrictJit) == sim::Card::Grass,
         "StrictJit lost the proven surplus-Grass fallback");
  expect(cost_for(sim::DciProfile::MatchupFlexJit) == sim::Card::Grass,
         "MatchupFlexJit did not share the same surplus-Grass fallback");
}

void test_no_discard_control_stays_outside_jit_fallback() {
  expect(cost_for(sim::DciProfile::NoDiscardControl) != sim::Card::Grass,
         "NoDiscardControl incorrectly entered the same-turn-JIT fallback");
}

void test_route_boundaries_remain_required() {
  const sim::Scenario scenario{"issue-3151-boundary",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 3};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3152);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state = route_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::treasure_cost(engine),
         "shared-JIT fallback ignored the spent attachment window");

  state = route_state();
  state.hand = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::treasure_cost(engine),
         "shared-JIT fallback spent the final required Grass");
}

}  // namespace

int main() {
  test_both_same_turn_jit_profiles_spend_only_surplus_grass();
  test_no_discard_control_stays_outside_jit_fallback();
  test_route_boundaries_remain_required();
  return 0;
}

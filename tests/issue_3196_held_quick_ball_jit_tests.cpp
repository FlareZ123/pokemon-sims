#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool held_quick_ball_route(const Engine& engine) {
    const std::vector<Card> wanted{Card::RegidragoVstar};
    return engine.steven_held_routes_complete_next_turn(
        wanted, false, true);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::QuickBall,
                sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::TapuLeleGX, sim::Card::MegaDragonite,
                sim::Card::Dragapult};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::CrobatV,
                sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::Arven, sim::Card::Serena,
                  sim::Card::MysteriousTreasure, sim::Card::Grass,
                  sim::Card::Fire, sim::Card::FieldBlower};
  return state;
}

bool route_visible(const sim::DciProfile dci) {
  const sim::Scenario scenario{"issue-3196-held-quick-ball", dci,
                               sim::LockMode::None, true, 3};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3196);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());
  return sim::EngineTestAccess::held_quick_ball_route(engine);
}

void test_same_ready_turn_profiles_share_route() {
  // Quick Ball discards exactly one card and searches a Basic Pokémon. Here its
  // paid discard is a held Dragon payload, while the held Fire is the single
  // manual Energy that completes Regidrago V's GGF cost after evolution:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Same-ready-turn StrictJit and MatchupFlexJit policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Earliest complete-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3196
  expect(route_visible(sim::DciProfile::StrictJit),
         "StrictJit lost the held Quick Ball payload route");
  expect(route_visible(sim::DciProfile::MatchupFlexJit),
         "MatchupFlexJit suppressed the same-ready-turn held Quick Ball route");
  expect(!route_visible(sim::DciProfile::NoDiscardControl),
         "NoDiscardControl incorrectly inherited same-ready-turn payload timing");
}

}  // namespace

int main() {
  test_same_ready_turn_profiles_share_route();
  return 0;
}

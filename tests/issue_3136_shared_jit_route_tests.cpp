#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_2272_route_replaced_arven_quick_ball_available();
  }

  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

sim::State route_replaced_arven_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::FieldBlower, sim::Card::Arven,
                sim::Card::PathToPeak, sim::Card::BrilliantBlender,
                sim::Card::MysteriousTreasure, sim::Card::QuickBall};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::StevensResolve, sim::Card::Crispin,
                   sim::Card::Serena, sim::Card::Dragapult};
  return state;
}

void test_matchup_flex_uses_the_shared_jit_route() {
  sim::Scenario scenario{"issue-3136/matchup-flex", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{3136};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::set_state(engine, route_replaced_arven_state());

  // Strict JIT and matchup-flex JIT share the repository's same-turn payload
  // contract. Quick Ball can therefore spend route-replaced Arven to search the
  // Basic Latias ex while held Brilliant Blender independently supplies the Dragon
  // payload for Apex Dragon in this otherwise identical K1 state.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Ability, and Retreat procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Shared JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed systemic overfit: https://github.com/FlareZ123/pokemon-sims/issues/3136
  if (!sim::EngineTestAccess::route_available(engine)) {
    throw std::runtime_error("#3136 matchup-flex state was rejected by the shared-JIT route.");
  }
  if (!sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("#3136 matchup-flex Quick Ball route did not play.");
  }

  const sim::State& state = sim::EngineTestAccess::state(engine);
  if (std::find(state.discard.begin(), state.discard.end(), sim::Card::Arven) ==
      state.discard.end()) {
    throw std::runtime_error("#3136 route did not spend route-replaced Arven.");
  }
  if (std::find(state.hand.begin(), state.hand.end(), sim::Card::LatiasEx) ==
      state.hand.end()) {
    throw std::runtime_error("#3136 route did not search Latias ex.");
  }
}

void test_no_discard_control_does_not_borrow_the_jit_route() {
  sim::Scenario scenario{"issue-3136/no-discard", sim::DciProfile::NoDiscardControl,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{3137};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::set_state(engine, route_replaced_arven_state());

  // No-discard-control does not use the same-turn JIT payload contract, so this
  // special route-conditioned Arven cost remains unavailable.
  // Shared JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed systemic overfit boundary: https://github.com/FlareZ123/pokemon-sims/issues/3136
  if (sim::EngineTestAccess::route_available(engine)) {
    throw std::runtime_error("#3136 JIT-only Arven cost leaked into no-discard-control.");
  }
}

}  // namespace

int main() {
  test_matchup_flex_uses_the_shared_jit_route();
  test_no_discard_control_does_not_borrow_the_jit_route();
}

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
    engine.prizes_revealed_ = true;
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_2289_quick_ball_tapu_arven_route_available();
  }

  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario route_scenario(const sim::DciProfile dci,
                             const bool going_first,
                             const int max_turn = 5) {
  return sim::Scenario{"issue-2634", dci, sim::LockMode::None,
                       going_first, max_turn};
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::Fire, sim::Card::Klara,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Arven,
                sim::Card::ForestSealStone, sim::Card::EarthenVessel,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::RegidragoV};
  state.prizes = {sim::Card::Crispin};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void current_jit_route_is_state_generic() {
  // Quick Ball's printed cost/search, Wonder Tag, Arven, Forest Seal Stone, and
  // Earthen Vessel depend on the current game state. Their legality does not depend
  // on which player started or a hard-coded turn label. The repository JIT contract
  // gives both Strict JIT and matchup-flex JIT the same same-turn payload rule.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Current-turn JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2634
  struct Case {
    sim::DciProfile dci;
    bool going_first;
    int turn;
  };
  const Case cases[] = {
      {sim::DciProfile::StrictJit, true, 2},
      {sim::DciProfile::StrictJit, false, 3},
      {sim::DciProfile::MatchupFlexJit, true, 4},
      {sim::DciProfile::MatchupFlexJit, false, 5},
  };

  std::uint64_t seed = 263401;
  for (const Case& test_case : cases) {
    std::mt19937_64 rng{seed++};
    const sim::Scenario scenario =
        route_scenario(test_case.dci, test_case.going_first, 5);
    sim::Engine engine = make_engine(scenario, rng, route_state(test_case.turn));
    expect(sim::EngineTestAccess::route_available(engine),
           "issue-2634 rejected an equivalent legal current-JIT state");
  }
}

void generalized_route_executes_in_matchup_flex_go_second() {
  std::mt19937_64 rng{263405};
  const sim::Scenario scenario =
      route_scenario(sim::DciProfile::MatchupFlexJit, false, 5);
  sim::Engine engine = make_engine(scenario, rng, route_state(3));

  // The Dragon paid to Quick Ball is DCI-1 on this ready turn because it supplies
  // the required current-turn Apex Dragon payload while the connector obtains every
  // remaining VSTAR/Energy resource. This is the same legal route as strict JIT.
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2634
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "issue-2634 generalized matchup-flex connector did not execute");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::MegaDragonite) == 1,
         "issue-2634 did not establish the current-turn Dragon payload");
  expect(after.supporter_used,
         "issue-2634 did not consume the single Supporter action on Arven");
  expect(std::count(after.hand.begin(), after.hand.end(),
                    sim::Card::ForestSealStone) == 1,
         "issue-2634 did not preserve the Forest Seal Stone finish");
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::Grass) == 1,
         "issue-2634 did not preserve the Earthen Vessel Grass finish");
}

void real_legality_gates_still_block() {
  // No-discard-control does not impose the current-turn JIT payload contract, and
  // the dedicated connector must still stop after the scenario's modeled horizon.
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/2634
  {
    std::mt19937_64 rng{263406};
    const sim::Scenario scenario =
        route_scenario(sim::DciProfile::NoDiscardControl, false, 5);
    sim::Engine engine = make_engine(scenario, rng, route_state(3));
    expect(!sim::EngineTestAccess::route_available(engine),
           "issue-2634 admitted the current-JIT connector outside current-JIT profiles");
  }
  {
    std::mt19937_64 rng{263407};
    const sim::Scenario scenario =
        route_scenario(sim::DciProfile::StrictJit, false, 3);
    sim::Engine engine = make_engine(scenario, rng, route_state(4));
    expect(!sim::EngineTestAccess::route_available(engine),
           "issue-2634 admitted a route after the modeled turn horizon");
  }
  {
    std::mt19937_64 rng{263408};
    const sim::Scenario scenario =
        route_scenario(sim::DciProfile::StrictJit, false, 5);
    sim::State state = route_state(3);
    state.supporter_used = true;
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine),
           "issue-2634 ignored Supporter contention");
  }
}

}  // namespace

int main() {
  current_jit_route_is_state_generic();
  generalized_route_executes_in_matchup_flex_go_second();
  real_legality_gates_still_block();
  return 0;
}

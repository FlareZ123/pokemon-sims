#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool proactive_tapu_attachment(const Engine& engine) {
    return engine.issue_1845_proactive_tapu_attachment_available();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State public_surplus_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Serena, sim::Card::MysteriousTreasure};
  return state;
}

bool admits(const sim::Scenario& scenario, sim::State state) {
  std::mt19937_64 rng{2987};
  const auto recipe = sim::deck_by_id("regidrago-shell")->recipe;
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::proactive_tapu_attachment(engine);
}

void equivalent_public_states_ignore_scenario_coordinates() {
  // The same visible resource state remains a legal dominance-safe Tapu bank in
  // other DCI profiles, lock profiles, seats, turns, and simulation horizons.
  // Those scenario labels do not alter Tapu's one-Colorless Retreat Cost or
  // Regidrago VSTAR's printed GGF requirement:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official attachment and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Refined public-state policy: https://github.com/FlareZ123/pokemon-sims/issues/1845#issuecomment-5123772411
  // Confirmed systemic overfit: https://github.com/FlareZ123/pokemon-sims/issues/2987
  const sim::Scenario variants[] = {
      {"issue-2987-flex-second", sim::DciProfile::MatchupFlexJit,
       sim::LockMode::None, false, 2},
      {"issue-2987-control-first", sim::DciProfile::NoDiscardControl,
       sim::LockMode::TurnTwoItem, true, 3},
  };
  for (const sim::Scenario& scenario : variants) {
    expect(admits(scenario, public_surplus_state(2)),
           "Equivalent public Tapu state was rejected by scenario coordinates");
  }
}

void actual_resource_and_route_guards_still_control() {
  const sim::Scenario scenario{"issue-2987-negative", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 3};

  sim::State only_two = public_surplus_state(2);
  only_two.hand.erase(only_two.hand.begin());
  expect(!admits(scenario, std::move(only_two)),
         "Tapu bank consumed the two-Grass attacker reserve");

  sim::State used_attachment = public_surplus_state(2);
  used_attachment.manual_energy_used = true;
  expect(!admits(scenario, std::move(used_attachment)),
         "Tapu bank ignored an already-used manual attachment");

  sim::State used_retreat = public_surplus_state(2);
  used_retreat.retreat_used = true;
  expect(!admits(scenario, std::move(used_retreat)),
         "Tapu bank ignored an already-used Retreat");

  sim::State stronger_regi = public_surplus_state(2);
  stronger_regi.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                                             sim::Tool::None});
  expect(!admits(scenario, std::move(stronger_regi)),
         "Tapu bank preempted an immediate Regidrago attachment route");

  sim::State stronger_switch = public_surplus_state(2);
  stronger_switch.hand.push_back(sim::Card::LatiasEx);
  expect(!admits(scenario, std::move(stronger_switch)),
         "Tapu bank preempted a superior Latias free-retreat route");
}

}  // namespace

int main() {
  try {
    equivalent_public_states_ignore_scenario_coordinates();
    actual_resource_and_route_guards_still_control();
  } catch (const std::exception& error) {
    std::cerr << "issue-2987 Tapu public-state test failure: " << error.what()
              << '\n';
    return 1;
  }
  return 0;
}

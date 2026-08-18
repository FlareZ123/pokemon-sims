#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool issue_1795_route_visible(const Engine& engine) {
    return engine.issue_1795_crispin_steven_vessel_route_available();
  }
};
}  // namespace sim

namespace {

sim::State known_route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MegaDragonite, sim::Card::Serena,
                sim::Card::StevensResolve, sim::Card::Gladion,
                sim::Card::Crispin, sim::Card::TapuLeleGX};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0,
                              sim::Tool::None},
                 sim::Pokemon{sim::Card::TapuLeleGX, turn - 1, 0, 0,
                              sim::Tool::None}};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::QuickBall, sim::Card::MysteriousTreasure};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven,
                  sim::Card::Guzma, sim::Card::Dragapult,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool visible(const sim::DciProfile dci, const sim::LockMode lock,
             const bool going_first, const int turn, const int max_turn) {
  const sim::Scenario scenario{"sol-1795-semantic-probe", dci, lock,
                               going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(1795);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, known_route_state(turn));
  return sim::EngineTestAccess::issue_1795_route_visible(engine);
}

}  // namespace

int main() {
  std::cout << "strict_go_first_none_t2="
            << visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                       true, 2, 4)
            << '\n';
  std::cout << "matchup_go_first_none_t2="
            << visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None,
                       true, 2, 4)
            << '\n';
  std::cout << "strict_go_second_none_t2="
            << visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                       false, 2, 4)
            << '\n';
  std::cout << "strict_go_first_rulebox_t2="
            << visible(sim::DciProfile::StrictJit,
                       sim::LockMode::FullRuleBoxAbility, true, 2, 4)
            << '\n';
  std::cout << "strict_go_first_none_t3="
            << visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                       true, 3, 5)
            << '\n';
  return 0;
}

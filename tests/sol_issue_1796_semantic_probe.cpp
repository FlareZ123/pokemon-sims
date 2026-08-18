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

  static bool issue_1796_route_visible(const Engine& engine) {
    return engine.issue_1796_t2_steven_route_available();
  }
};
}  // namespace sim

namespace {

sim::State known_route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, turn - 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::Gladion, sim::Card::TapuLeleGX};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Fire};
  state.manual_energy_used = true;
  return state;
}

bool visible(const sim::DciProfile dci, const bool going_first,
             const int turn, const int max_turn) {
  const sim::Scenario scenario{"sol-1796-semantic-probe", dci,
                               sim::LockMode::None, going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(1796);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, known_route_state(turn));
  return sim::EngineTestAccess::issue_1796_route_visible(engine);
}

}  // namespace

int main() {
  std::cout << "strict_go_first_t2="
            << visible(sim::DciProfile::StrictJit, true, 2, 3) << '\n';
  std::cout << "matchup_go_first_t2="
            << visible(sim::DciProfile::MatchupFlexJit, true, 2, 3) << '\n';
  std::cout << "strict_go_second_t2="
            << visible(sim::DciProfile::StrictJit, false, 2, 3) << '\n';
  std::cout << "strict_go_first_t3="
            << visible(sim::DciProfile::StrictJit, true, 3, 4) << '\n';
  return 0;
}

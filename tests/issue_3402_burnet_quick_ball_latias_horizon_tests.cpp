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
    return engine.issue_2343_burnet_quick_ball_latias_route_available();
  }
};
}

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State complete_route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, turn - 1, 2, 0, sim::Tool::ForestSealStone},
      sim::Pokemon{sim::Card::TapuLeleGX, turn, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::QuickBall, sim::Card::StevensResolve, sim::Card::Fire};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Crispin, sim::Card::LatiasEx,
                sim::Card::MegaDragonite, sim::Card::Dragapult, sim::Card::Grass};
  return state;
}

bool available(const int turn, const int max_turn, sim::State state) {
  state.turn = turn;
  sim::Scenario scenario{"issue-3402", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, max_turn};
  std::mt19937_64 rng{3402};
  sim::Engine engine{scenario, sim::baseline_recipe(), rng};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::route_available(engine);
}

void test_equivalent_current_states_use_relative_horizon() {
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/3402
  expect(available(2, 4, complete_route_state(2)), "T2 route rejected.");
  expect(available(3, 4, complete_route_state(3)), "T3 route rejected.");
  expect(available(4, 4, complete_route_state(4)), "T4 route rejected.");
  expect(!available(4, 3, complete_route_state(4)), "Expired-horizon route admitted.");
}

void test_existing_latias_gate_remains_required() {
  sim::State state = complete_route_state(2);
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::LatiasEx), state.deck.end());
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/2343
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/3402
  expect(!available(2, 4, std::move(state)), "Missing-Latias route admitted.");
}
}

int main() {
  test_equivalent_current_states_use_relative_horizon();
  test_existing_latias_gate_remains_required();
  return 0;
}

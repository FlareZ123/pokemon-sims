#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_ultra_ball_prefers_tapu_over_irrelevant_fallback() {
  const sim::Scenario scenario{"ultra-ball-tapu", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2718};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::RegidragoV,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::LatiasEx, sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball can search for any Pokémon after discarding two other hand cards:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (!sim::EngineTestAccess::play_ultra_ball(engine, false)) {
    throw std::runtime_error("Ultra Ball should take the live Tapu Lele-GX connector.");
  }

  const sim::State& after_ball = sim::EngineTestAccess::state(engine);
  if (!contains(after_ball.hand, sim::Card::TapuLeleGX) || contains(after_ball.hand, sim::Card::LatiasEx)) {
    throw std::runtime_error("Ultra Ball must select Tapu Lele-GX before an irrelevant Latias ex fallback.");
  }

  // Wonder Tag resolves when Tapu Lele-GX is played from hand onto the Bench and
  // can search the live Crispin Supporter: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should have Bench space for Wonder Tag.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Crispin)) {
    throw std::runtime_error("Wonder Tag should complete the Ultra Ball to Crispin connector.");
  }
}

}  // namespace

int main() {
  try {
    test_ultra_ball_prefers_tapu_over_irrelevant_fallback();
    std::cout << "ultra ball Tapu connector tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

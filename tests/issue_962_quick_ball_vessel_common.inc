#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool run_search_step(Engine& engine, const bool permit_payload) {
    return engine.run_search_items_one_step(permit_payload);
  }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::MysteriousTreasure,
      sim::Card::DialgaGX,
      sim::Card::Serena,
      sim::Card::EarthenVessel,
      sim::Card::BrilliantBlender,
      sim::Card::QuickBall,
      sim::Card::GoodraVstar,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::LatiasEx,
      sim::Card::Oricorio,
  };
  return state;
}

[[maybe_unused]] void require_vessel_was_first(const sim::State& state) {
  if (!contains(state.discard, sim::Card::EarthenVessel) ||
      contains(state.discard, sim::Card::QuickBall) ||
      !contains(state.hand, sim::Card::QuickBall)) {
    throw std::runtime_error("The negative control must keep Quick Ball and spend Earthen Vessel first.");
  }
}

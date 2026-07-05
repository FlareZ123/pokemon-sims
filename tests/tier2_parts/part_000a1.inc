#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <functional>
#include <iostream>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static bool prizes_known(const Engine& engine) { return engine.prizes_known(); }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_mysterious_treasure(Engine& engine, bool payload) { return engine.play_mysterious_treasure(payload); }
  static bool play_quick_ball(Engine& engine, bool payload) { return engine.play_quick_ball(payload); }
  static bool play_ultra_ball(Engine& engine, bool payload) { return engine.play_ultra_ball(payload); }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }

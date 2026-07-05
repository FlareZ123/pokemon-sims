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
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool bench_from_hand(Engine& engine, Card card, bool entry) { return engine.bench_from_hand(card, entry); }
  static bool play_earthen_vessel(Engine& engine, bool payload) { return engine.play_earthen_vessel(payload); }
  static bool play_brilliant_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool evolve_best_regi(Engine& engine) { return engine.evolve_best_regi(); }
};
}

namespace {
using namespace sim;

struct Fixture {
  DeckRecipe recipe{baseline_recipe()};
  Scenario scenario;
  std::mt19937_r4` rng{7331};
  TraceLog trace{true, {}};
  Engine engine;
  Fixture(Scenario sc, State s) : scenario(std::move(sc)), engine(scenario, recipe, rng, &trace) {
    EngineTestAccess::set_state(engine, std::move(s));
  }
};

Scenario sc(std::string label, DciProfile profile = DciProfile::StrictJit, LockMode locks = LockMode::None, bool first = false) {
  return Scenario{std::move(label), profile, locks, first, 4};
}
State st(int turn, Card active = Card::RegidragoV, int entered = 0, int grass = 0, int fire = 0) {
  State s; s.turn = turn; s.active = Pokemon{active, entered, grass, fire, Tool::None}; return s;
}
bool has(const std::vector<Card>& cards, Card card) { return std::find(cards.begin(), cards.end(), card) != cards.end(); }
void expect(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }

void crispin_beats_arven() {
  State s = st(2, Card::RegidragoV, 1, 2, 0);
  s.hand = {Card::Crispin, Card::Arven, Card::RegidragoVstar};
  s.deck = {Card::Grass, Card::Fire, Card::EarthenVessel, Card::ForestSealStone};
  Fixture f(sc("tier2-crispin-over-arven"), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::Crispin), "Crispin should be selected before Arven.");
  expect(has(a.hand, Card::Arven), "Arven should be preserved.");
  expect(a.active->grass == 2 && a.active->fire == 1, "Crispin should complete GGF now.");
  expect(EngineTestAccess::evolve_best_regi(f.engine), "The powered prior-turn V should evolve now.");
}

void steven_beats_arven() {
  State s = st(1, Card::RegidragoV, 0);
  s.hand = {Card::StevensResolve, Card::Arven};
  s.deck = {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender, Card::ForestSealStone};
  Fixture f(sc("tier2-steven-over-arven", DciProfile::StrictJit, LockMode::None, false), s);
  EngineTestAccess::choose_supporter(f.engine);
  const State& a = EngineTestAccess::state(f.engine);
  expect(has(a.discard, Card::StevensResolve), "Steven should be selected before Arven.");
  expect(has(a.hand, Card::RegidragoVstar) && has(a.hand, Card::Crispin) && has(a.hand, Card::BrilliantBlender), "Steven should cover VSTAR, Energy, and payload.");
  expect(has(a.hand, Card::Arven) && a.turn_ended, "Arven should remain and Steven should end the turn.");
}

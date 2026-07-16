#define REGIDRAGO_SIM_NO_MAIN
#include "../../src/regidrago_sim.cpp"

#include <functional>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool value) { engine.deck_seen_ = value; }
  static State& state(Engine& engine) { return engine.state_; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static TrialOutcome& outcome(Engine& engine) { return engine.outcome_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
  static bool play_lusamine(Engine& engine) { return engine.play_lusamine(); }
  static bool play_earthen_vessel(Engine& engine, bool permit_payload) { return engine.play_earthen_vessel(permit_payload); }
  static bool play_mysterious_treasure(Engine& engine, bool permit_payload) { return engine.play_mysterious_treasure(permit_payload); }
  static bool play_quick_ball(Engine& engine, bool permit_payload) { return engine.play_quick_ball(permit_payload); }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool bench_oricorio_if_useful(Engine& engine) { return engine.bench_oricorio_if_useful(); }
  static bool bench_tapu_if_useful(Engine& engine) { return engine.bench_tapu_if_useful(); }
  static bool bench_from_hand(Engine& engine, Card card, bool resolve_entry) { return engine.bench_from_hand(card, resolve_entry); }
  static bool retreat_to_benched_vstar_with_latias(Engine& engine) { return engine.retreat_to_benched_vstar_with_latias(); }
  static bool can_free_retreat_with_latias(const Engine& engine) { return engine.can_free_retreat_with_latias(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool recover_discard_to_hand(Engine& engine, Card card) { return engine.recover_discard_to_hand(card); }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool use_celestial_roar(Engine& engine) { return engine.use_celestial_roar(); }
  static bool evolve_best_regi(Engine& engine) { return engine.evolve_best_regi(); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool item_locked(const Engine& engine) { return engine.item_locked(); }
  static bool supporter_allowed(const Engine& engine) { return engine.supporter_allowed(); }
  static bool play_brilliant_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool run_search_items_one_step(Engine& engine, bool permit_payload) { return engine.run_search_items_one_step(permit_payload); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool setup_axis_missing(const Engine& engine) { return engine.setup_axis_missing(); }
};

}  // namespace sim

namespace {
using sim::Card;
using sim::DciProfile;
using sim::Engine;
using sim::EngineTestAccess;
using sim::LockMode;
using sim::Pokemon;
using sim::Scenario;
using sim::State;
using sim::Tool;
using sim::TraceLog;

struct Fixture {
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  Scenario scenario;
  std::mt19937_64 rng{77};
  TraceLog trace{true, {}};
  Engine engine;

  Fixture(Scenario value, State state) : scenario(std::move(value)), engine(scenario, recipe, rng, &trace) {
    EngineTestAccess::set_state(engine, std::move(state));
  }
};

Scenario scenario(std::string label = "fixture", DciProfile dci = DciProfile::StrictJit,
                  LockMode locks = LockMode::None, bool first = false) {
  return Scenario{std::move(label), dci, locks, first, 4};
}

State state(int turn, Card active = Card::RegidragoV, int entered_turn = 0, int grass = 0, int fire = 0) {
  State result;
  result.turn = turn;
  result.active = Pokemon{active, entered_turn, grass, fire, Tool::None};
  return result;
}

bool contains(const std::vector<Card>& cards, Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<Card>& cards, Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void expect(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

void test_gladion_fetches_prized_vstar_only_when_no_route_exists() {
  State s = state(1);
  s.hand = {Card::Gladion};
  s.prizes = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::MawileGX, Card::Guzma};
  Fixture f(scenario("gladion-collapse"), s);
  expect(EngineTestAccess::play_gladion(f.engine), "Gladion should be selected when VSTAR has no other route.");
  const State& after = EngineTestAccess::state(f.engine);

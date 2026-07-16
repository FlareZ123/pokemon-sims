#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <string>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool payload_ready(Engine& engine) { return engine.payload_ready(); }
  static bool need_payload(Engine& engine) { return engine.need_payload(); }
  static bool can_play_payload_this_turn(Engine& engine) {
    return engine.can_play_payload_this_turn();
  }
  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

using namespace sim;

Scenario strict_scenario(const std::string& label = "strict-jit-test") {
  return Scenario{label, DciProfile::StrictJit, LockMode::None, false, 4};
}

bool trace_contains(const TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

int trace_count(const TraceLog& trace, const std::string& needle) {
  return static_cast<int>(std::count_if(
      trace.lines.begin(), trace.lines.end(),
      [&needle](const std::string& line) {
        return line.find(needle) != std::string::npos;
      }));
}

void test_prior_turn_payload_remains_ready() {
  std::mt19937_64 rng(1);
  Engine engine(strict_scenario("prior-turn-payload"), baseline_recipe(), rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 3;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn.clear();

  assert(EngineTestAccess::payload_ready(engine));
  assert(!EngineTestAccess::need_payload(engine));
}

void test_incomplete_attacker_keeps_strict_window_closed() {
  std::mt19937_64 rng(2);
  Engine engine(strict_scenario("incomplete-attacker"), baseline_recipe(), rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::LatiasEx, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 2, 0, Tool::None}};
  state.hand = {Card::RegidragoVstar, Card::BrilliantBlender, Card::Fire};
  state.deck = {Card::MegaDragonite};
  state.manual_energy_used = true;

  assert(!EngineTestAccess::can_play_payload_this_turn(engine));
}

void test_secured_attacker_opens_strict_window() {
  std::mt19937_64 rng(3);
  Engine engine(strict_scenario("secured-attacker"), baseline_recipe(), rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 3;
  state.active = Pokemon{Card::LatiasEx, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None}};
  state.hand = {Card::RegidragoVstar, Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite};

  assert(EngineTestAccess::can_play_payload_this_turn(engine));
}

void test_existing_payload_does_not_trigger_legacy_star() {
  std::mt19937_64 rng(4);
  Engine engine(strict_scenario("legacy-star-hold"), baseline_recipe(), rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 3;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.discard = {Card::MegaDragonite, Card::BrilliantBlender};
  state.deck = {Card::Dragapult, Card::Grass, Card::Fire, Card::MawileGX,
                Card::Guzma, Card::Channeler, Card::FieldBlower};

  assert(!EngineTestAccess::use_legacy_star(engine));
  assert(!state.vstar_power_used);
}

void test_same_turn_treasure_to_vstar_route_remains_legal() {
  std::mt19937_64 rng(5);
  Engine engine(strict_scenario("treasure-to-vstar"), baseline_recipe(), rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::MysteriousTreasure, Card::MegaDragonite};
  state.deck = {Card::RegidragoVstar};

  EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(std::find(state.discard.begin(), state.discard.end(),
                   Card::MegaDragonite) != state.discard.end());
  assert(EngineTestAccess::payload_ready(engine));
}

void test_seed_three_uses_one_blender_and_no_legacy_star() {
  const auto scenario = scenario_by_label("strict-jit/go-second");
  assert(scenario.has_value());

  std::mt19937_64 rng(3);
  TraceLog trace;
  trace.enabled = true;
  Engine engine(*scenario, baseline_recipe(), rng, &trace);
  const TrialOutcome outcome = engine.run();

  assert(outcome.first_ready_turn == 3);
  assert(!outcome.used_legacy_star);
  assert(trace_count(trace, "R-BLENDER-01") == 1);
  assert(!trace_contains(trace, "T2 | PLAY ITEM | rules: R-BLENDER-01"));
  assert(!trace_contains(trace, "LEGACY STAR"));
  assert(trace_contains(trace, "T3 | READY"));
  assert(trace_contains(trace, "payload already in discard remains ready"));
}

}  // namespace

int main() {
  test_prior_turn_payload_remains_ready();
  test_incomplete_attacker_keeps_strict_window_closed();
  test_secured_attacker_opens_strict_window();
  test_existing_payload_does_not_trigger_legacy_star();
  test_same_turn_treasure_to_vstar_route_remains_legal();
  test_seed_three_uses_one_blender_and_no_legacy_star();
  return 0;
}

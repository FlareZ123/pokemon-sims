#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <string>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
};

}  // namespace sim

namespace {

int hold_trace_count(const sim::TraceLog& trace) {
  return static_cast<int>(std::count_if(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("HOLD VSTAR POWER") != std::string::npos;
      }));
}

void set_k1_dead_target_state(sim::Engine& engine, const sim::Tool tool) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, tool};
  state.deck = {sim::Card::Serena};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_no_tool_emits_no_star_alchemy_hold_trace() {
  using namespace sim;
  const Scenario scenario{"fss-hold-trace-no-tool", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(72701);
  TraceLog trace;
  trace.enabled = true;
  Engine engine(scenario, recipe, rng, &trace);
  set_k1_dead_target_state(engine, Tool::None);

  // Forest Seal Stone grants Star Alchemy only to the Pokemon V it is attached to:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // A readable trace must report only resources present in the public game state:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // Regression scope: https://github.com/FlareZ123/pokemon-sims/issues/727
  assert(!EngineTestAccess::use_fss(engine));
  assert(hold_trace_count(trace) == 0);
  assert(!EngineTestAccess::state(engine).vstar_power_used);
}

void test_live_tool_preserves_star_alchemy_hold_trace() {
  using namespace sim;
  const Scenario scenario{"fss-hold-trace-live-tool", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(72702);
  TraceLog trace;
  trace.enabled = true;
  Engine engine(scenario, recipe, rng, &trace);
  set_k1_dead_target_state(engine, Tool::ForestSealStone);

  // An attached Forest Seal Stone supplies the one-per-game Star Alchemy resource,
  // so the K1 dead-target policy may truthfully report that it was retained:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Regression scope: https://github.com/FlareZ123/pokemon-sims/issues/727
  assert(!EngineTestAccess::use_fss(engine));
  assert(hold_trace_count(trace) == 1);
  assert(!EngineTestAccess::state(engine).vstar_power_used);
}

}  // namespace

int main() {
  test_no_tool_emits_no_star_alchemy_hold_trace();
  test_live_tool_preserves_star_alchemy_hold_trace();
  return 0;
}

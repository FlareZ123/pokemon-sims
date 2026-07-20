#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_ready(const Engine& engine) {
    return engine.latias_serena_completion_ready();
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State exact_state() {
  sim::State state;
  state.turn = 5;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::LatiasEx, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::Gladion};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dragapult};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Lusamine,
                  sim::Card::Grass, sim::Card::RegidragoV,
                  sim::Card::QuickBall, sim::Card::MysteriousTreasure};
  state.discard = {sim::Card::Arven};
  return state;
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::FullItem) {
  return sim::Scenario{"issue-1126-latias-serena", sim::DciProfile::StrictJit,
                       locks, true, 5};
}

bool route_ready_for(sim::State state,
                     const sim::LockMode locks = sim::LockMode::FullItem) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1126};
  sim::Engine engine(scenario(locks), recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::route_ready(engine);
}

void test_route_blocks_gladion() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1126};
  sim::Engine engine(scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_state());

  // Serena's mandatory Dragon discard establishes the strict-JIT payload, while
  // Latias ex gives the Basic Active Dialga-GX no Retreat Cost and promotes the
  // prepared GGF Regidrago VSTAR. Gladion advances neither remaining axis:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Bench, Ability, Supporter, discard, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-ready route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1126
  expect(sim::EngineTestAccess::route_ready(engine),
         "The exact Latias-Serena route must be recognized.");
  // Gladion's isolated preflight must honor the same public completion route.
  expect(!sim::EngineTestAccess::play_gladion(engine),
         "Gladion must yield to the current-turn Latias-Serena completion.");
}

void test_turn_policy_completes_both_axes() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1126};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario(), recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, exact_state());

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Serena must consume the Supporter play.");
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Skyliner must promote the prepared Regidrago VSTAR.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Serena must establish the current-turn Dragon payload.");
  expect(trace_contains(trace, "Mega Dragonite ex"),
         "The trace must record Serena's Dragon discard.");
  expect(!trace_contains(trace, "exchanged Gladion"),
         "The trace must not exchange Gladion.");
}

void test_negative_controls() {
  sim::State state = exact_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::MegaDragonite), state.hand.end());
  expect(!route_ready_for(state), "A modeled Dragon payload is required.");

  state = exact_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::LatiasEx), state.hand.end());
  expect(!route_ready_for(state), "Latias ex must be available.");

  state = exact_state();
  state.bench = {state.bench.front(),
                 sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None}};
  expect(!route_ready_for(state), "A held Latias ex needs an open Bench slot.");

  state = exact_state();
  state.bench.front().grass = 1;
  expect(!route_ready_for(state), "The Benched VSTAR must already pay GGF.");

  state = exact_state();
  state.retreat_used = true;
  expect(!route_ready_for(state), "The retreat action must remain unused.");

  state = exact_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0,
                              sim::Tool::None};
  expect(!route_ready_for(state), "Skyliner requires a Basic Active.");

  expect(!route_ready_for(exact_state(), sim::LockMode::FullCombined),
         "Rule Box Ability lock must disable Skyliner.");
}
}  // namespace

int main() {
  test_route_blocks_gladion();
  test_turn_policy_completes_both_axes();
  test_negative_controls();
  return 0;
}

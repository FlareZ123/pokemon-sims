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
  }
  static std::optional<Card> bridge_payload(const Engine& engine) {
    return engine.late_steven_payload_treasure_bridge_payload();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
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

sim::State issue_1111_state(const bool include_blender) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure};
  if (include_blender) state.hand.push_back(sim::Card::BrilliantBlender);
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV,
                sim::Card::Grass};
  return state;
}

sim::Scenario issue_1111_scenario() {
  return sim::Scenario{"issue-1111-held-blender", sim::DciProfile::StrictJit,
                       sim::LockMode::None, false, 5};
}

void test_held_blender_blocks_late_steven_bridge() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1111};
  sim::Engine engine(issue_1111_scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1111_state(true));

  // Brilliant Blender directly searches and discards a Dragon payload this turn.
  // Steven's Resolve would end the turn before the already-held Item can resolve:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1111
  expect(!sim::EngineTestAccess::bridge_payload(engine),
         "A live held Blender must suppress the slower late Steven bridge.");
}

void test_turn_policy_plays_blender_and_reaches_ready_state() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1112};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(issue_1111_scenario(), recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, issue_1111_state(true));

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  const sim::TrialOutcome& outcome = sim::EngineTestAccess::outcome(engine);

  // The one-action Item route preserves the Supporter play and establishes the
  // strict-JIT payload on the current ready turn:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1111
  expect(outcome.used_blender, "The exact state must use Brilliant Blender.");
  expect(!outcome.used_steven, "The exact state must not use Steven's Resolve.");
  expect(!after.supporter_used,
         "The direct Blender route must leave the Supporter play unused.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Blender must establish the current-turn strict-JIT payload.");
  expect(trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01"),
         "The exact trace must record the T3 Blender play.");
}

void test_bridge_remains_available_without_blender() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1113};
  sim::Engine engine(issue_1111_scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1111_state(false));

  // Preserve the confirmed payload-to-Mysterious-Treasure continuation when the
  // faster Blender outlet is absent:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  // https://github.com/FlareZ123/pokemon-sims/issues/1111
  expect(sim::EngineTestAccess::bridge_payload(engine) ==
             sim::Card::MegaDragonite,
         "The late Steven bridge must remain live when Blender is absent.");
}
}  // namespace

int main() {
  test_held_blender_blocks_late_steven_bridge();
  test_turn_policy_plays_blender_and_reaches_ready_state();
  test_bridge_remains_available_without_blender();
  return 0;
}

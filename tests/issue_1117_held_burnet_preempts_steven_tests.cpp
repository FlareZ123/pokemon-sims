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

sim::State issue_1117_state(const bool include_burnet) {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure};
  if (include_burnet) state.hand.push_back(sim::Card::ProfessorBurnet);
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::RegidragoV};
  return state;
}

sim::Scenario issue_1117_scenario() {
  return sim::Scenario{"issue-1117-held-burnet", sim::DciProfile::StrictJit,
                       sim::LockMode::None, false, 5};
}

void test_held_burnet_blocks_late_steven_bridge() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario scenario = issue_1117_scenario();
  std::mt19937_64 rng{1117};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1117_state(true));

  // Burnet directly searches and discards a permitted payload this turn. Steven's
  // Resolve ends the turn before that current-turn completion can occur:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1117
  expect(!sim::EngineTestAccess::bridge_payload(engine),
         "A live held Burnet must suppress the slower late Steven bridge.");
}

void test_turn_policy_plays_burnet_and_reaches_ready_state() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario scenario = issue_1117_scenario();
  std::mt19937_64 rng{1118};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, issue_1117_state(true));

  sim::EngineTestAccess::run_turn(engine);
  const sim::TrialOutcome& outcome = sim::EngineTestAccess::outcome(engine);

  // The direct Supporter route establishes the strict-JIT payload on the current
  // ready turn and avoids Steven's mandatory turn ending effect:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1117
  expect(!outcome.used_steven, "The exact state must not use Steven's Resolve.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Burnet must establish the current-turn strict-JIT payload.");
  expect(trace_contains(trace, "T4 | PLAY SUPPORTER | rules: R-BURNET-01"),
         "The exact trace must record the T4 Burnet play.");
}

void test_bridge_remains_available_without_burnet() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario scenario = issue_1117_scenario();
  std::mt19937_64 rng{1119};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1117_state(false));

  // Preserve the confirmed Steven-to-Mysterious-Treasure continuation when the
  // faster Burnet outlet is absent:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  // https://github.com/FlareZ123/pokemon-sims/issues/1117
  expect(sim::EngineTestAccess::bridge_payload(engine) ==
             sim::Card::MegaDragonite,
         "The late Steven bridge must remain live when Burnet is absent.");
}
}  // namespace

int main() {
  test_held_burnet_blocks_late_steven_bridge();
  test_turn_policy_plays_burnet_and_reaches_ready_state();
  test_bridge_remains_available_without_burnet();
  return 0;
}

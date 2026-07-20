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

sim::State issue_1115_state(const bool include_burnet) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  if (include_burnet) state.hand.push_back(sim::Card::ProfessorBurnet);
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  state.prizes = {sim::Card::Lusamine, sim::Card::Fire, sim::Card::Grass,
                  sim::Card::RegidragoV, sim::Card::QuickBall,
                  sim::Card::MysteriousTreasure};
  state.discard = {sim::Card::Arven};
  return state;
}

sim::Scenario issue_1115_scenario() {
  return sim::Scenario{"issue-1115-held-burnet", sim::DciProfile::StrictJit,
                       sim::LockMode::None, true, 5};
}

void test_live_burnet_blocks_gladion_lusamine_exchange() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1115};
  sim::Engine engine(issue_1115_scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1115_state(true));

  // Burnet completes the only missing current-turn axis. Gladion into Lusamine
  // consumes the same one-per-turn Supporter permission without advancing setup:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1115
  expect(!sim::EngineTestAccess::play_gladion(engine),
         "Gladion must yield to a live Active-VSTAR Burnet completion.");
}

void test_turn_policy_plays_burnet_on_turn_three() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1116};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(issue_1115_scenario(), recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, issue_1115_state(true));

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);

  // The selected route uses Burnet and reaches strict-JIT readiness immediately:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  expect(after.supporter_used, "Burnet must consume the T3 Supporter play.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Burnet must establish the T3 payload.");
  expect(trace_contains(trace, "Professor Burnet"),
         "The trace must record Professor Burnet.");
  expect(!trace_contains(trace, "exchanged Gladion"),
         "The trace must not exchange Gladion for Lusamine.");
}

void test_gladion_remains_available_without_burnet() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1117};
  sim::Engine engine(issue_1115_scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1115_state(false));

  // Preserve the existing known-Prize recovery route when the direct Burnet outlet
  // is absent:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://github.com/FlareZ123/pokemon-sims/issues/1115
  expect(sim::EngineTestAccess::play_gladion(engine),
         "Gladion must remain legal without the held Burnet completion.");
}
}  // namespace

int main() {
  test_live_burnet_blocks_gladion_lusamine_exchange();
  test_turn_policy_plays_burnet_on_turn_three();
  test_gladion_remains_available_without_burnet();
  return 0;
}

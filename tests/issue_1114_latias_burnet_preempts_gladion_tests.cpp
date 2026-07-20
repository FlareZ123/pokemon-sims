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

sim::State issue_1114_state(const bool include_latias) {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Gladion};
  if (include_latias) state.hand.push_back(sim::Card::LatiasEx);
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  state.prizes = {sim::Card::Lusamine, sim::Card::Fire, sim::Card::Grass,
                  sim::Card::RegidragoV, sim::Card::QuickBall,
                  sim::Card::MysteriousTreasure};
  state.discard = {sim::Card::Arven};
  return state;
}

sim::Scenario issue_1114_scenario() {
  return sim::Scenario{"issue-1114-latias-burnet", sim::DciProfile::StrictJit,
                       sim::LockMode::None, true, 5};
}

void test_latias_burnet_route_blocks_gladion_lusamine_exchange() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1114};
  const sim::Scenario scenario = issue_1114_scenario();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1114_state(true));

  // Latias ex removes Oricorio's Retreat Cost, allowing the prepared GGF VSTAR to
  // become Active before Professor Burnet completes the strict-JIT payload axis:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1114
  expect(!sim::EngineTestAccess::play_gladion(engine),
         "Gladion must yield to the current-turn Latias-Burnet completion.");
}

void test_turn_policy_promotes_and_plays_burnet_on_turn_four() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1114};
  sim::TraceLog trace;
  trace.enabled = true;
  const sim::Scenario scenario = issue_1114_scenario();
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, issue_1114_state(true));

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);

  expect(after.supporter_used, "Professor Burnet must consume the T4 Supporter play.");
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Latias ex must promote the prepared Benched Regidrago VSTAR.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Professor Burnet must establish the T4 payload.");
  expect(trace_contains(trace, "Latias ex"), "The trace must record Latias ex.");
  expect(trace_contains(trace, "Professor Burnet"),
         "The trace must record Professor Burnet.");
  expect(!trace_contains(trace, "exchanged Gladion"),
         "The trace must not exchange Gladion for Lusamine.");
}

void test_gladion_route_remains_without_latias_promotion() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1114};
  const sim::Scenario scenario = issue_1114_scenario();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1114_state(false));

  expect(sim::EngineTestAccess::play_gladion(engine),
         "Gladion must remain available without the Latias promotion route.");
}
}  // namespace

int main() {
  test_latias_burnet_route_blocks_gladion_lusamine_exchange();
  test_turn_policy_promotes_and_plays_burnet_on_turn_four();
  test_gladion_route_remains_without_latias_promotion();
  return 0;
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng, trace);
}

sim::State missing_regi_state(const sim::Card fallback) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.hand = {sim::Card::QuickBall, fallback, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_channeler_pays_exact_missing_regi_deadlock() {
  const sim::Scenario scenario{"issue-1018-channeler", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1018};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, missing_regi_state(sim::Card::Channeler), true);

  // Channeler only removes attack effects, and this setup model executes no opposing
  // attacks. Quick Ball may discard that currently inert singleton to find the exact
  // missing Basic Regidrago V axis:
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1018
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Channeler must pay Quick Ball in the exact strict-DCI Basic deadlock.");
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(contains(state.hand, sim::Card::RegidragoV),
         "Quick Ball must find Regidrago V.");
  expect(contains(state.discard, sim::Card::Channeler),
         "The currently inert Channeler must enter discard.");
  expect(contains(state.hand, sim::Card::Dragapult),
         "The protected Dragon payload must remain in hand.");
}

void test_team_yell_fallback_requires_no_live_recovery_target() {
  const sim::Scenario scenario{"issue-1018-team-yell-live", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1019};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = missing_regi_state(sim::Card::TeamYellsCheer);
  state.discard = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // Team Yell's Cheer has a modeled live purpose once Regidrago VSTAR is in discard,
  // so the singleton must remain protected:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1018
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "A live Team Yell's Cheer recovery route must block the fallback.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::TeamYellsCheer),
         "Live Team Yell's Cheer must remain in hand.");
}

void test_existing_lower_dci_duplicate_stays_ahead_of_channeler() {
  const sim::Scenario scenario{"issue-1018-duplicate-priority", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1020};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = missing_regi_state(sim::Card::Channeler);
  state.hand.push_back(sim::Card::QuickBall);
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // A second Quick Ball is already an ordinary lower-DCI cost and must remain ahead
  // of the singleton fallback: https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1018
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "The duplicate Quick Ball route must stay live.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::Channeler),
         "Channeler must be preserved when duplicate Item fodder exists.");
  expect(std::count(result.discard.begin(), result.discard.end(), sim::Card::QuickBall) == 2,
         "The played Quick Ball and duplicate cost must both enter discard.");
}

void test_no_missing_regi_axis_preserves_inert_singleton() {
  const sim::Scenario scenario{"issue-1018-no-regi-axis", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1021};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = missing_regi_state(sim::Card::Channeler);
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  // The fallback belongs only to Quick Ball establishing the exact missing Regidrago V
  // axis: https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/issues/1018
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "An established Regidrago V axis must preserve Channeler.");
}

void test_seed_70_uses_quick_ball_and_benches_regidrago() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{70};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  engine.run();

  // Exact source-bound reproduction: Quick Ball discards Channeler, searches the
  // missing Basic, benches Regidrago V, and advances the opening setup graph:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1018
  expect(trace_contains(trace, "T1 | DISCARD | rules: R-QB-01 | Channeler"),
         "Seed 70 must discard Channeler with Quick Ball on T1.");
  expect(trace_contains(trace, "T1 | PLAY ITEM | rules: R-QB-01; R-GAME-ITEM | Searched a Basic Pokémon: Regidrago V"),
         "Seed 70 must search Regidrago V on T1.");
  expect(trace_contains(trace, "T1 | BENCH | rules: R-GAME-BENCH | Regidrago V"),
         "Seed 70 must bench the searched Regidrago V on T1.");
}
}  // namespace

int main() {
  test_channeler_pays_exact_missing_regi_deadlock();
  test_team_yell_fallback_requires_no_live_recovery_target();
  test_existing_lower_dci_duplicate_stays_ahead_of_channeler();
  test_no_missing_regi_axis_preserves_inert_singleton();
  test_seed_70_uses_quick_ball_and_benches_regidrago();
  return 0;
}

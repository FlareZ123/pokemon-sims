#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State missing_regi_and_vstar_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::Channeler};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar};
  return state;
}

void test_channeler_cost_preserves_live_second_treasure() {
  const sim::Scenario scenario{"issue-1740-unit", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1740};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, missing_regi_and_vstar_state());

  // The first Treasure establishes Regidrago V. The second remains the shortest
  // observable connector to Regidrago VSTAR, while Channeler has no setup effect:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1740
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "The first Treasure did not use Channeler as its cost.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.hand, sim::Card::MysteriousTreasure) == 1,
         "The live second Treasure was not preserved.");
  expect(count(after.hand, sim::Card::RegidragoV) == 1,
         "The first Treasure did not search Regidrago V.");
  expect(count(after.discard, sim::Card::Channeler) == 1,
         "Channeler did not pay the first Treasure cost.");
  expect(count(after.discard, sim::Card::MysteriousTreasure) == 1,
         "More than the played Treasure entered discard.");
}

void test_negative_controls() {
  const sim::Scenario strict{"issue-1740-negative", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  {
    std::mt19937_64 rng{1741};
    sim::Engine engine(strict, recipe, rng);
    sim::State state = missing_regi_and_vstar_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                               sim::Card::Channeler));
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
           "The ordinary no-Channeler duplicate cost stopped working.");
    expect(count(sim::EngineTestAccess::state(engine).discard,
                 sim::Card::MysteriousTreasure) == 2,
           "The no-Channeler route did not spend the duplicate Treasure.");
  }

  {
    std::mt19937_64 rng{1742};
    sim::Engine engine(strict, recipe, rng);
    sim::State state = missing_regi_and_vstar_state();
    state.hand.push_back(sim::Card::RegidragoVstar);
    state.deck = {sim::Card::RegidragoV};
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
           "The held-VSTAR route stopped using an ordinary duplicate cost.");
    expect(count(sim::EngineTestAccess::state(engine).discard,
                 sim::Card::MysteriousTreasure) == 2,
           "A held VSTAR card incorrectly protected the duplicate Treasure.");
  }

  {
    std::mt19937_64 rng{1743};
    sim::Engine engine(strict, recipe, rng);
    sim::State state = missing_regi_and_vstar_state();
    state.hand.erase(state.hand.begin());
    const auto original_hand = state.hand;
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "A singleton Treasure incorrectly spent Channeler.");
    expect(sim::EngineTestAccess::state(engine).hand == original_hand,
           "The rejected singleton route changed the hand.");
  }

  {
    std::mt19937_64 rng{1744};
    sim::Engine engine(strict, recipe, rng);
    sim::State state = missing_regi_and_vstar_state();
    state.deck = {sim::Card::Arven};
    const auto original_hand = state.hand;
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
           "A known zero-target Treasure incorrectly paid a discard cost.");
    expect(sim::EngineTestAccess::state(engine).hand == original_hand,
           "The known zero-target route changed the hand.");
  }
}

void test_registered_seed_reaches_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1740 registered fixture is unavailable.");
  std::mt19937_64 rng{271828};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound seed must preserve the second Treasure for the T3 VSTAR
  // search and reach the earliest visible T4 promotion route:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Issue and original CI witness: https://github.com/FlareZ123/pokemon-sims/issues/1740 https://github.com/FlareZ123/pokemon-sims/actions/runs/30390502968
  expect(outcome.first_ready_turn == 4,
         "Seed 271828 did not improve from failure to T4 readiness.");
  expect(trace_contains(trace, "T1 | DISCARD") &&
             trace_contains(trace, "Channeler (Mysterious Treasure cost)") &&
             trace_contains(trace, "T3 | DISCARD") &&
             trace_contains(trace, "Mega Dragonite ex (Mysterious Treasure cost)") &&
             trace_contains(trace, "T3 | EVOLVE") &&
             trace_contains(trace, "T4 | READY"),
         "Seed 271828 omitted a required corrected-route action.");
}
}  // namespace

int main() {
  test_channeler_cost_preserves_live_second_treasure();
  test_negative_controls();
  test_registered_seed_reaches_t4();
}

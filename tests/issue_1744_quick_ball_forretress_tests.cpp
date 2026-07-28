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
  static bool route_available(const Engine& engine) {
    return engine.issue_1744_quick_ball_forretress_route_available();
  }
  static void run_secret_box_turn(Engine& engine) {
    engine.run_secret_box_turn_issue1744();
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

sim::State complete_quick_ball_forretress_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1}};
  state.hand = {sim::Card::SecretBox, sim::Card::ForretressEx,
                sim::Card::QuickBall, sim::Card::Dragapult,
                sim::Card::Arven, sim::Card::Crispin};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoV,
                sim::Card::Fire, sim::Card::Pineco};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (deck == nullptr) throw std::runtime_error("Pineco recipe is unavailable.");
  return sim::Engine(scenario, deck->recipe, rng, trace);
}

void test_quick_ball_route_preserves_secret_box() {
  const sim::Scenario scenario{"issue-1744-unit", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng{1744};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario, rng, &trace);
  sim::EngineTestAccess::set_state(engine, complete_quick_ball_forretress_state(), true);

  // Quick Ball needs one discard and can search a Basic. The discarded Dragon is
  // the current-turn Apex payload, while normal Pineco evolution and Exploding
  // Energy supply the final Grass without paying Secret Box's three-card cost:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1744
  expect(sim::EngineTestAccess::route_available(engine),
         "The complete issue-1744 route was not admitted.");
  sim::EngineTestAccess::run_secret_box_turn(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->grass == 2 && after.active->fire == 1,
         "Exploding Energy did not complete GGF.");
  expect(count(after.discard, sim::Card::Dragapult) == 1,
         "Quick Ball did not establish the current-turn Dragon payload.");
  expect(count(after.hand, sim::Card::SecretBox) == 1,
         "The equivalent route spent Secret Box.");
  expect(count(after.hand, sim::Card::Arven) == 1 &&
             count(after.hand, sim::Card::Crispin) == 1 &&
             !after.supporter_used,
         "The cheaper route failed to preserve Supporter resources.");
  expect(trace_contains(trace, "Quick Ball current-turn payload cost") &&
             trace_contains(trace, "preserved Secret Box") &&
             trace_contains(trace, "Exploding Energy"),
         "The issue-1744 trace omitted a required corrected action.");
}

void test_negative_controls() {
  const sim::Scenario flex{"issue-1744-negative", sim::DciProfile::MatchupFlexJit,
                           sim::LockMode::None, true, 4};
  const sim::Scenario locked{"issue-1744-locked", sim::DciProfile::MatchupFlexJit,
                             sim::LockMode::FullItem, true, 4};

  const auto rejected = [&](sim::State state, const sim::Scenario& scenario,
                            const bool k1, const std::uint64_t seed,
                            const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), k1);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  rejected(complete_quick_ball_forretress_state(), flex, false, 17440,
           "The K1-only route was admitted at K0.");
  rejected(complete_quick_ball_forretress_state(), locked, true, 17441,
           "Quick Ball was admitted through Item lock.");

  {
    sim::State state = complete_quick_ball_forretress_state();
    state.bench.front().entered_turn = state.turn;
    rejected(std::move(state), flex, true, 17442,
             "A same-turn Pineco was allowed to evolve.");
  }
  {
    sim::State state = complete_quick_ball_forretress_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                               sim::Card::Dragapult));
    rejected(std::move(state), flex, true, 17443,
             "The route was admitted without a held payload.");
  }
  {
    sim::State state = complete_quick_ball_forretress_state();
    state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(), sim::is_basic),
                     state.deck.end());
    rejected(std::move(state), flex, true, 17444,
             "Quick Ball was admitted without a Basic search target.");
  }
  {
    sim::State state = complete_quick_ball_forretress_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
                     state.deck.end());
    rejected(std::move(state), flex, true, 17445,
             "Exploding Energy was admitted without searchable Grass.");
  }
  {
    sim::State state = complete_quick_ball_forretress_state();
    state.active->grass = 0;
    rejected(std::move(state), flex, true, 17446,
             "The exact one-Grass completion admitted a two-Grass deficit.");
  }
}

void test_registered_seed_1618033_reaches_t3_without_secret_box() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1744 registered fixture is unavailable.");

  std::mt19937_64 rng{1618033};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound seed reaches T3 through Quick Ball, ordinary evolution, and
  // Exploding Energy while preserving the singleton ACE SPEC and both Supporters:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Issue and original CI witness: https://github.com/FlareZ123/pokemon-sims/issues/1744 https://github.com/FlareZ123/pokemon-sims/actions/runs/30394042881
  expect(outcome.first_ready_turn == 3,
         "Seed 1618033 did not retain T3 readiness.");
  expect(trace_contains(trace, "Quick Ball current-turn payload cost") &&
             trace_contains(trace, "Exploding Energy") &&
             trace_contains(trace, "T3 | READY") &&
             !trace_contains(trace, "T3 | SECRET BOX"),
         "Seed 1618033 did not take the cheaper corrected route.");
}
}  // namespace

int main() {
  test_quick_ball_route_preserves_secret_box();
  test_negative_controls();
  test_registered_seed_1618033_reaches_t3_without_secret_box();
}

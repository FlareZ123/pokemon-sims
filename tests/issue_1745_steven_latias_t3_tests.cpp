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
    return engine.issue_1745_steven_latias_t3_route_available();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven_issue1745();
  }
  static bool play_steven(Engine& engine) {
    return engine.play_steven_issue1745();
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

sim::State complete_steven_latias_state() {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::EarthenVessel, sim::Card::QuickBall,
                sim::Card::Fire, sim::Card::StevensResolve,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Arven, sim::Card::Pineco};
  state.discard = {sim::Card::MegaDragonite, sim::Card::EarthenVessel,
                   sim::Card::Arven};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  if (deck == nullptr) throw std::runtime_error("Crobat modeling recipe is unavailable.");
  return sim::Engine(scenario, deck->recipe, rng, trace);
}

void test_steven_searches_complete_t3_package() {
  const sim::Scenario scenario{"issue-1745-unit", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1745};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario, rng, &trace);
  sim::EngineTestAccess::set_state(engine, complete_steven_latias_state(), true);

  // Steven can search all three missing cards and then ends T2. On T3 the
  // prior-turn Regidrago evolves, Grass completes GGF, Latias gives the Basic
  // Active zero Retreat Cost, and the banked Dragon remains a legal payload:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, no-discard-control, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1745
  expect(sim::EngineTestAccess::route_available(engine),
         "The complete issue-1745 route was not admitted.");
  expect(sim::EngineTestAccess::should_play_steven(engine),
         "The Steven selector rejected the deterministic package.");
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven did not resolve the issue-1745 route.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.turn_ended && after.supporter_used,
         "Steven did not end T2 after resolving.");
  expect(count(after.hand, sim::Card::RegidragoVstar) == 1 &&
             count(after.hand, sim::Card::LatiasEx) == 1 &&
             count(after.hand, sim::Card::Grass) == 1,
         "Steven did not search the exact three-card T3 package.");
  expect(trace_contains(trace, "deterministic T3 Latias route"),
         "The corrected Steven trace was not emitted.");
}

void test_negative_controls() {
  const sim::Scenario no_control{"issue-1745-negative",
                                  sim::DciProfile::NoDiscardControl,
                                  sim::LockMode::None, false, 5};
  const sim::Scenario locked{"issue-1745-locked",
                              sim::DciProfile::NoDiscardControl,
                              sim::LockMode::FullRuleBoxAbility, false, 5};
  const sim::Scenario strict{"issue-1745-strict", sim::DciProfile::StrictJit,
                              sim::LockMode::None, false, 5};

  const auto rejected = [&](sim::State state, const sim::Scenario& scenario,
                            const bool k1, const std::uint64_t seed,
                            const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), k1);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  rejected(complete_steven_latias_state(), no_control, false, 17450,
           "The K1-only Steven route was admitted at K0.");
  rejected(complete_steven_latias_state(), locked, true, 17451,
           "The Latias route was admitted through Rule Box Ability lock.");
  rejected(complete_steven_latias_state(), strict, true, 17452,
           "The banked-payload route was admitted under strict JIT.");

  {
    sim::State state = complete_steven_latias_state();
    state.manual_energy_used = false;
    rejected(std::move(state), no_control, true, 17453,
             "The post-attachment route was admitted before the T2 attachment.");
  }
  {
    sim::State state = complete_steven_latias_state();
    state.discard.clear();
    rejected(std::move(state), no_control, true, 17454,
             "The route was admitted without a banked payload.");
  }
  {
    sim::State state = complete_steven_latias_state();
    state.bench.front().entered_turn = state.turn;
    rejected(std::move(state), no_control, true, 17455,
             "A same-turn Regidrago was treated as evolution-eligible.");
  }
  {
    sim::State state = complete_steven_latias_state();
    for (int copy = 0; copy < 4; ++copy) {
      state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1});
    }
    rejected(std::move(state), no_control, true, 17456,
             "Latias was admitted with a full Bench.");
  }
  {
    sim::State state = complete_steven_latias_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::RegidragoVstar),
                     state.deck.end());
    rejected(std::move(state), no_control, true, 17457,
             "Steven was admitted without a VSTAR target.");
  }
  {
    sim::State state = complete_steven_latias_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::LatiasEx),
                     state.deck.end());
    rejected(std::move(state), no_control, true, 17458,
             "Steven was admitted without Latias ex.");
  }
  {
    sim::State state = complete_steven_latias_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Grass),
                     state.deck.end());
    rejected(std::move(state), no_control, true, 17459,
             "Steven was admitted without the final Grass.");
  }
}

void test_crobat_seed_1234567_reaches_t3() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-second");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1745 modeling fixture is unavailable.");

  std::mt19937_64 rng{1234567};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound K1 state has all three Steven targets outside Prizes. The
  // searched package produces T3 evolution, Grass attachment, Latias bench,
  // Skyliner retreat, and readiness without depending on the T3 draw:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Issue and original CI witness: https://github.com/FlareZ123/pokemon-sims/issues/1745 https://github.com/FlareZ123/pokemon-sims/actions/runs/30394042881
  expect(outcome.first_ready_turn == 3,
         "Crobat seed 1234567 did not improve from failure to T3 readiness.");
  expect(trace_contains(trace, "deterministic T3 Latias route") &&
             trace_contains(trace, "T3 | EVOLVE") &&
             trace_contains(trace, "T3 | BENCH") &&
             trace_contains(trace, "T3 | RETREAT") &&
             trace_contains(trace, "T3 | READY"),
         "Crobat seed 1234567 omitted a required corrected-route action.");
}
}  // namespace

int main() {
  test_steven_searches_complete_t3_package();
  test_negative_controls();
  test_crobat_seed_1234567_reaches_t3();
}

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1771_steven_t4_package_available();
  }
  static bool play_steven(Engine& engine) {
    return engine.play_issue_1771_steven_t4_package();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State exact_t3_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::TateLiza,
                sim::Card::ChaoticSwell, sim::Card::PathToPeak,
                sim::Card::Grass};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::ProfessorBurnet, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::QuickBall, sim::Card::QuickBall,
                   sim::Card::Crispin};
  return state;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1771", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) throw std::runtime_error("Registered shell is unavailable.");
  return sim::Engine(selected, deck->recipe, rng, trace);
}

void test_exact_package_is_selected() {
  sim::Scenario selected = scenario();
  std::mt19937_64 rng{1771};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(selected, rng, &trace);
  sim::EngineTestAccess::set_state(engine, exact_t3_state());

  // The K1 state proves all three Steven targets. They cover evolution, Active
  // promotion, and a current-turn strict-JIT payload on T4, while Tate draw mode
  // would randomize the complete route:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1771
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact K1 Steven package was rejected.");
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven did not resolve the deterministic package.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.turn_ended && after.supporter_used,
         "Steven did not consume the Supporter play and end the turn.");
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::RegidragoVstar) == 1 &&
             std::count(after.hand.begin(), after.hand.end(), sim::Card::LatiasEx) == 1 &&
             std::count(after.hand.begin(), after.hand.end(), sim::Card::ProfessorBurnet) == 1,
         "Steven did not search the exact three-card package.");
  expect(trace_contains(trace, "deterministic T4 VSTAR-Latias-Burnet package"),
         "The corrected route trace was not emitted.");
}

void test_route_gates() {
  const auto rejected = [](sim::State state, sim::Scenario selected,
                           const bool k1, const std::uint64_t seed,
                           const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(selected, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), k1);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  rejected(exact_t3_state(), scenario(), false, 17710,
           "The K1-only route was admitted at K0.");
  rejected(exact_t3_state(), scenario(sim::LockMode::FullRuleBoxAbility), true,
           17711, "The Latias route bypassed Rule Box Ability lock.");
  {
    sim::State state = exact_t3_state();
    state.bench.back().entered_turn = state.turn;
    rejected(std::move(state), scenario(), true, 17712,
             "A same-turn Regidrago was treated as next-turn prepared.");
  }
  {
    sim::State state = exact_t3_state();
    state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1});
    state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1});
    state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1});
    rejected(std::move(state), scenario(), true, 17713,
             "The route was admitted without Latias Bench space.");
  }
  for (const sim::Card missing : {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                                  sim::Card::ProfessorBurnet,
                                  sim::Card::Dragapult}) {
    sim::State state = exact_t3_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), missing),
                     state.deck.end());
    rejected(std::move(state), scenario(), true,
             17720 + static_cast<std::uint64_t>(missing),
             "The route ignored a missing required deck target.");
  }
  {
    sim::State state = exact_t3_state();
    state.bench.back().grass = 1;
    rejected(std::move(state), scenario(), true, 17730,
             "The route was admitted without GGF.");
  }
}

void test_registered_seed_reaches_t4() {
  const auto selected = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1771 fixture is unavailable.");
  std::mt19937_64 rng{905};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Seed 905 must replace the former T3 Tate shuffle with the observable Steven
  // package, then evolve, Bench Latias, use Burnet, retreat, and reach T4:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/issues/1771
  expect(outcome.first_ready_turn == 4,
         "Registered seed 905 did not improve from failure to T4 readiness.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "deterministic T4 VSTAR-Latias-Burnet package") &&
             !trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-TATE-01") &&
             trace_contains(trace, "T4 | EVOLVE") &&
             trace_contains(trace, "T4 | BENCH") &&
             trace_contains(trace, "T4 | PLAY SUPPORTER") &&
             trace_contains(trace, "T4 | RETREAT") &&
             trace_contains(trace, "T4 | READY"),
         "The seed-905 trace omitted a required corrected-route action.");
}
}  // namespace

int main() {
  test_exact_package_is_selected();
  test_route_gates();
  test_registered_seed_reaches_t4();
}

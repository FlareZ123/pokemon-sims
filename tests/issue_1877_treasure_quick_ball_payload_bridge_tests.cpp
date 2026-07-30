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
    engine.prizes_revealed_ = k1;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1877_treasure_quick_ball_payload_bridge_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1877_treasure_quick_ball_payload_bridge();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::FieldBlower, sim::Card::MysteriousTreasure,
                sim::Card::EarthenVessel, sim::Card::ErikasInvitation,
                sim::Card::QuickBall, sim::Card::Oricorio};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV,
                sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::StevensResolve, sim::Card::Grass,
                  sim::Card::TapuLeleGX, sim::Card::Dipplin,
                  sim::Card::Crispin, sim::Card::Serena};
  state.discard = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV};
  return state;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1877", sim::DciProfile::StrictJit, lock, false, 5};
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        const char* deck_id = "regidrago-shell",
                        sim::TraceLog* trace = nullptr) {
  const sim::NamedDeck* deck = sim::deck_by_id(deck_id);
  if (deck == nullptr) throw std::runtime_error("Registered deck is unavailable.");
  return sim::Engine(selected, deck->recipe, rng, trace);
}

void test_exact_k1_route_completes_t3() {
  std::mt19937_64 rng{1877};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario(), rng, "regidrago-shell", &trace);
  sim::EngineTestAccess::set_state(engine, exact_t3_state());

  // The Active VSTAR already has GGF. Earthen Vessel is route-replaced, so it
  // pays Mysterious Treasure. Treasure finds a permitted Dragon, and Quick Ball
  // discards that Dragon during the strict-JIT ready turn before searching a
  // legal Basic that may remain in hand:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard-cost, search, shuffle, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1877
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact K1 Treasure-to-Quick-Ball bridge was rejected.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact K1 Treasure-to-Quick-Ball bridge did not complete.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "The corrected route changed the complete Active attacker.");
  expect(std::find(after.discarded_this_turn.begin(),
                   after.discarded_this_turn.end(),
                   sim::Card::MegaDragonite) != after.discarded_this_turn.end(),
         "The searched Dragon was not recorded as the current-turn payload.");
  expect(std::find(after.hand.begin(), after.hand.end(),
                   sim::Card::RegidragoV) != after.hand.end(),
         "Quick Ball did not place its legal Basic target into hand.");
  expect(trace_contains(trace, "route-replaced Earthen Vessel") &&
             trace_contains(trace, "current-turn payload") &&
             trace_contains(trace, "Basic remains in hand"),
         "The corrected trace omitted the bridge's DCI, JIT, or Basic-search action.");
}

void test_route_gates_and_full_bench_boundary() {
  const auto rejected = [](sim::State state, sim::Scenario selected,
                           const bool k1, const char* deck_id,
                           const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(selected, rng, deck_id);
    sim::EngineTestAccess::set_state(engine, std::move(state), k1);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  rejected(exact_t3_state(), scenario(), false, "regidrago-shell", 18770,
           "The K1-only bridge was admitted at K0.");
  rejected(exact_t3_state(), scenario(sim::LockMode::FullItem), true,
           "regidrago-shell", 18771, "The bridge bypassed Item lock.");
  rejected(exact_t3_state(), scenario(), true, "regidrago-pineco", 18772,
           "The shell-only bridge displaced the Pineco policy.");

  for (const sim::Card missing : {sim::Card::MysteriousTreasure,
                                  sim::Card::EarthenVessel,
                                  sim::Card::QuickBall}) {
    sim::State state = exact_t3_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), missing),
                     state.hand.end());
    rejected(std::move(state), scenario(), true, "regidrago-shell",
             18780 + static_cast<std::uint64_t>(missing),
             "The bridge ignored a missing required Item.");
  }
  {
    sim::State state = exact_t3_state();
    state.active->grass = 1;
    rejected(std::move(state), scenario(), true, "regidrago-shell", 18790,
             "The payload-only bridge ignored an incomplete Energy axis.");
  }
  {
    sim::State state = exact_t3_state();
    state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                    sim::is_payload),
                     state.deck.end());
    rejected(std::move(state), scenario(), true, "regidrago-shell", 18791,
             "The bridge ignored the absence of a permitted Dragon target.");
  }
  {
    sim::State state = exact_t3_state();
    state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                    sim::is_basic),
                     state.deck.end());
    rejected(std::move(state), scenario(), true, "regidrago-shell", 18792,
             "The bridge ignored the absence of a Quick Ball Basic target.");
  }
  {
    sim::State state = exact_t3_state();
    state.bench = {
        sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
        sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
        sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None},
        sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
        sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None}};
    std::mt19937_64 rng{18793};
    sim::Engine engine = make_engine(scenario(), rng);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::route_available(engine),
           "A full Bench incorrectly blocked a Quick Ball target that remains in hand.");
  }
}

void test_registered_seed_reaches_t3() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1877 fixture is unavailable.");
  std::mt19937_64 rng{169};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Registered seed 169 did not improve from T4 to T3 readiness.");
  expect(trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "T3 | QUICK BALL") &&
             trace_contains(trace, "T3 | READY") &&
             !trace_contains(trace, "T4 | BRILLIANT BLENDER"),
         "The registered trace omitted the corrected T3 bridge or retained the T4 route.");
}
}  // namespace

int main() {
  test_exact_k1_route_completes_t3();
  test_route_gates_and_full_bench_boundary();
  test_registered_seed_reaches_t3();
}

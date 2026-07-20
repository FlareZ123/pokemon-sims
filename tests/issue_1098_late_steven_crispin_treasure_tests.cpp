#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static bool route_available(const Engine& engine) {
    return engine.late_steven_active_vstar_crispin_treasure_route_available();
  }
  static bool play_route(Engine& engine) {
    return engine.play_late_steven_active_vstar_crispin_treasure_route();
  }
  static void begin_turn(Engine& engine, const int turn) { engine.begin_turn(turn); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::State pre_steven_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Crispin, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::RegidragoV, sim::Card::LatiasEx,
                sim::Card::FieldBlower};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_k1_route_admission_and_exact_targets() {
  const sim::Scenario scenario{"issue-1098-exact", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  std::mt19937_64 rng{109801};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario, rng, pre_steven_state(), true, &trace);

  // K1 proves the two searched cards and the post-Steven Treasure target. Steven
  // banks Crispin plus Grass, ends T2, and preserves the held Dragon as the T3
  // strict-JIT discard cost:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official turn and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Policy specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1098
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact K1 late-Steven route must be admitted.");
  expect(sim::EngineTestAccess::play_route(engine),
         "The exact late-Steven route must resolve.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.turn_ended, "Steven's Resolve must end the turn.");
  expect(after.active && after.active->grass == 0 && after.active->fire == 1,
         "No post-Steven manual attachment may occur on T2.");
  expect(count_card(after.hand, sim::Card::Crispin) == 1,
         "Steven must search exactly one Crispin.");
  expect(count_card(after.hand, sim::Card::Grass) == 1,
         "Steven must search exactly one Grass Energy.");
  expect(count_card(after.hand, sim::Card::MegaDragonite) == 1,
         "The held Dragon payload must remain available for Mysterious Treasure.");
  expect(trace_contains(trace, "Searched the late Crispin-Treasure route: Crispin, Grass Energy"),
         "The trace must identify the issue-1098 target package.");
}

void test_route_boundaries() {
  const sim::Scenario scenario{"issue-1098-boundaries", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  const auto blocked = [&](sim::State state, const bool deck_seen,
                           const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state), deck_seen);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  blocked(pre_steven_state(), false, 109802,
          "K0 must not assume the route's exact deck contents.");

  sim::State one_grass = pre_steven_state();
  one_grass.deck.erase(std::find(one_grass.deck.begin(), one_grass.deck.end(), sim::Card::Grass));
  one_grass.deck.erase(std::find(one_grass.deck.begin(), one_grass.deck.end(), sim::Card::Grass));
  blocked(std::move(one_grass), true, 109803,
          "Steven must leave a second Grass for Crispin after searching one.");

  sim::State no_fire = pre_steven_state();
  no_fire.deck.erase(std::find(no_fire.deck.begin(), no_fire.deck.end(), sim::Card::Fire));
  blocked(std::move(no_fire), true, 109804,
          "Crispin must retain two different Basic Energy types.");

  sim::State no_target = pre_steven_state();
  no_target.deck.erase(std::remove_if(no_target.deck.begin(), no_target.deck.end(),
      [](const sim::Card card) { return card == sim::Card::RegidragoV || card == sim::Card::LatiasEx; }),
      no_target.deck.end());
  blocked(std::move(no_target), true, 109805,
          "Mysterious Treasure must retain a legal Psychic or Dragon target.");

  sim::State no_payload = pre_steven_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(), sim::Card::MegaDragonite));
  blocked(std::move(no_payload), true, 109806,
          "The strict-JIT Dragon discard must already be held.");

  const sim::Scenario item_lock{"issue-1098-item-lock", sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, true, 3};
  std::mt19937_64 lock_rng{109807};
  sim::Engine locked = make_engine(item_lock, lock_rng, pre_steven_state());
  expect(!sim::EngineTestAccess::route_available(locked),
         "Item lock must block the Mysterious Treasure continuation.");
}

void test_single_fire_topdeck_boundary() {
  const sim::Scenario scenario{"issue-1098-topdeck", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};

  sim::State success;
  success.turn = 2;
  success.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1};
  success.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                  sim::Card::Crispin, sim::Card::Grass};
  success.deck = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::FieldBlower};
  std::mt19937_64 success_rng{109808};
  sim::TraceLog success_trace{true, {}};
  sim::Engine success_engine = make_engine(scenario, success_rng, success, true, &success_trace);
  sim::EngineTestAccess::begin_turn(success_engine, 3);
  sim::EngineTestAccess::run_turn(success_engine);
  sim::EngineTestAccess::record_ready(success_engine);
  expect(sim::EngineTestAccess::outcome(success_engine).first_ready_turn == 3,
         "A non-Fire topdeck must preserve the T3 Crispin-Treasure completion.");

  sim::State failure = success;
  failure.deck = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire};
  std::mt19937_64 failure_rng{109809};
  sim::TraceLog failure_trace{true, {}};
  sim::Engine failure_engine = make_engine(scenario, failure_rng, failure, true, &failure_trace);
  sim::EngineTestAccess::begin_turn(failure_engine, 3);
  sim::EngineTestAccess::run_turn(failure_engine);
  sim::EngineTestAccess::record_ready(failure_engine);
  expect(sim::EngineTestAccess::outcome(failure_engine).first_ready_turn == 0,
         "Drawing the lone Fire must remove Crispin's second searchable Energy type.");
  expect(trace_contains(failure_trace, "start-of-turn: Fire Energy"),
         "The negative control must draw the lone Fire Energy.");
}

void test_seed_44_uses_steven_and_reaches_turn_three() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{44};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Seed 44 must retain its earliest source-bound T3 readiness.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(trace, "Searched the late Crispin-Treasure route"),
         "Seed 44 must choose Steven's 41/42 route on T2.");
  expect(!trace_contains(trace, "T2 | ATTACH |"),
         "Steven must prevent every later T2 attachment.");
  expect(trace_contains(trace, "T3 | DISCARD | rules: R-MT-01 | Mega Dragonite ex"),
         "Seed 44 must establish the Dragon payload through Mysterious Treasure on T3.");
  expect(trace_contains(trace, "T3 | READY |"),
         "Seed 44 must record T3 readiness.");
}
}  // namespace

int main() {
  try {
    test_k1_route_admission_and_exact_targets();
    test_route_boundaries();
    test_single_fire_topdeck_boundary();
    test_seed_44_uses_steven_and_reaches_turn_three();
    std::cout << "Issue 1098 late-Steven Crispin-Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

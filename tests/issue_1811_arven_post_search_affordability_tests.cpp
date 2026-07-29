#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool arven(Engine& engine) { return engine.play_arven(); }
  static bool treasure(Engine& engine) { return engine.play_mysterious_treasure(false); }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
};
}  // namespace sim

namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool has(const std::vector<sim::Card>& cards, sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_has(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&text](const std::string& line) {
    return line.find(text) != std::string::npos;
  });
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0, sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Arven, sim::Card::Fire, sim::Card::EarthenVessel,
                sim::Card::Dragapult, sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
                sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  return state;
}

sim::Engine engine_for(const sim::Scenario& scenario, std::mt19937_64& rng,
                       sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void complete_route_test() {
  const sim::Scenario scenario{"issue-1811", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  std::mt19937_64 rng{181101};
  sim::Engine engine = engine_for(scenario, rng, route_state());

  // Arven establishes K1. Fire becomes dynamic-DCI fuel because T2 Crispin replaces
  // both Energy roles, while Earthen Vessel plus Dragapult ex preserves strict JIT:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI/JIT, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1811
  expect(sim::EngineTestAccess::arven(engine), "Arven must resolve.");
  expect(has(sim::EngineTestAccess::state(engine).hand, sim::Card::MysteriousTreasure),
         "Arven must search Mysterious Treasure.");
  expect(has(sim::EngineTestAccess::state(engine).hand, sim::Card::ForestSealStone),
         "Arven must search Forest Seal Stone.");
  expect(sim::EngineTestAccess::treasure(engine), "Treasure must use replaced Fire.");
  expect(count(sim::EngineTestAccess::state(engine).discard, sim::Card::Fire) == 1,
         "Treasure must discard one Fire.");
  expect(has(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar),
         "Treasure must search Regidrago VSTAR.");
  expect(sim::EngineTestAccess::attach_fss(engine), "Forest Seal Stone must attach.");
  expect(sim::EngineTestAccess::use_fss(engine), "Star Alchemy must search Crispin.");
  expect(has(sim::EngineTestAccess::state(engine).hand, sim::Card::Crispin),
         "Crispin must be held after Star Alchemy.");
}

void missing_axes_test() {
  const sim::Scenario scenario{"issue-1811-negative", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  const auto rejects = [&](sim::State state, std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = engine_for(scenario, rng, std::move(state));
    sim::EngineTestAccess::arven(engine);
    expect(!has(sim::EngineTestAccess::state(engine).hand, sim::Card::MysteriousTreasure),
           message);
  };

  sim::State state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
  rejects(std::move(state), 181102, "Held Fire is required.");
  state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::EarthenVessel));
  rejects(std::move(state), 181103, "Held Vessel is required.");
  state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Dragapult));
  rejects(std::move(state), 181104, "Held payload is required.");
  for (const auto [missing, seed] : {
           std::pair{sim::Card::Crispin, UINT64_C(181105)},
           std::pair{sim::Card::Grass, UINT64_C(181106)},
           std::pair{sim::Card::Fire, UINT64_C(181107)}}) {
    state = route_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), missing));
    rejects(std::move(state), seed, "Both Crispin and its Energy targets are required.");
  }
}

void generic_cost_priority_test() {
  const sim::Scenario scenario{"issue-1811-priority", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  sim::State state = route_state();
  state.hand.push_back(sim::Card::HisuianHeavyBall);
  std::mt19937_64 rng{181108};
  sim::Engine engine = engine_for(scenario, rng, std::move(state));
  expect(sim::EngineTestAccess::arven(engine), "Arven must resolve with generic fuel.");
  expect(sim::EngineTestAccess::treasure(engine), "Treasure must use generic fuel.");
  // K1 proves Heavy Ball setup-dead when no Basic is prized, so it stays ahead of Fire:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  expect(count(sim::EngineTestAccess::state(engine).discard, sim::Card::HisuianHeavyBall) == 1,
         "Lower-DCI Heavy Ball must pay Treasure first.");
  expect(count(sim::EngineTestAccess::state(engine).discard, sim::Card::Fire) == 0,
         "Fire must remain when generic fuel exists.");
}

void exact_seed_test() {
  const sim::Scenario scenario{"strict-jit/go-second", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{424242};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Source-bound regression and confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1811
  expect(outcome.first_ready_turn == 2, "Seed 424242 must improve from T5 to T2.");
  expect(trace_has(trace, "Mysterious Treasure, Forest Seal Stone"),
         "Arven must take both route channels.");
  expect(trace_has(trace, "route-replaced Fire Energy"),
         "Treasure must spend replaced Fire.");
  expect(trace_has(trace, "Searched Crispin for the deterministic T2"),
         "Star Alchemy must bank Crispin.");
  expect(trace_has(trace, "T2 | DISCARD | rules: R-EV-01 | Dragapult ex"),
         "Vessel must establish the T2 payload.");
  expect(trace_has(trace, "T2 | READY |"), "Trace must record T2 readiness.");
}
}  // namespace

int main() {
  try {
    complete_route_test();
    missing_axes_test();
    generic_cost_priority_test();
    exact_seed_test();
    std::cout << "Issue 1811 Arven post-search affordability tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

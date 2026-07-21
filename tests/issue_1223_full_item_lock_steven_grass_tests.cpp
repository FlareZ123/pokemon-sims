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
  static bool route_available(const Engine& engine) {
    return engine.issue_1223_full_item_lock_steven_grass_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::Scenario full_item_scenario(const sim::LockMode locks = sim::LockMode::FullItem) {
  return sim::Scenario{"issue-1223", sim::DciProfile::StrictJit, locks, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  state.hand = {sim::Card::StevensResolve, sim::Card::ProfessorBurnet,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::ForestSealStone, sim::Card::LatiasEx,
                sim::Card::MegaDragonite, sim::Card::Dragapult};
  return state;
}

sim::Engine make_engine(const sim::Scenario& chosen, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(chosen, recipe, rng, nullptr);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_exact_route_and_blockers() {
  const sim::Scenario chosen = full_item_scenario();
  std::mt19937_64 rng{122301};
  sim::Engine live = make_engine(chosen, rng, route_state());

  // The one-shot Steven search must spend its third target on the missing Energy
  // channel because Regidrago VSTAR is already held. Crispin then preserves the
  // two-type attachment schedule through permanent Item lock:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1223
  expect(sim::EngineTestAccess::route_available(live),
         "The exact full-Item-lock Steven Grass route must be available.");

  const auto unavailable = [&rng](sim::State state, const sim::Scenario scenario,
                                  const char* message) {
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  sim::State no_vstar = route_state();
  erase_one(no_vstar.hand, sim::Card::RegidragoVstar);
  unavailable(no_vstar, full_item_scenario(),
              "A missing held VSTAR must preserve direct VSTAR search.");

  sim::State one_grass = route_state();
  erase_one(one_grass.deck, sim::Card::Grass);
  unavailable(one_grass, full_item_scenario(),
              "One Grass cannot fund Steven plus Crispin.");

  sim::State one_type = route_state();
  erase_one(one_type.deck, sim::Card::Fire);
  unavailable(one_type, full_item_scenario(),
              "Crispin needs two different Basic Energy types.");

  sim::State full_bench = route_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Dipplin, 1});
  unavailable(full_bench, full_item_scenario(),
              "One remaining Bench slot cannot hold Regidrago V and Latias ex.");

  unavailable(route_state(), full_item_scenario(sim::LockMode::FullRuleBoxAbility),
              "Rule Box Ability lock must block Skyliner.");

  sim::State no_stone = route_state();
  erase_one(no_stone.deck, sim::Card::ForestSealStone);
  unavailable(no_stone, full_item_scenario(),
              "Missing Forest Seal Stone must block the projected route.");

  sim::State no_latias = route_state();
  erase_one(no_latias.deck, sim::Card::LatiasEx);
  unavailable(no_latias, full_item_scenario(),
              "Missing Latias ex must block the promotion route.");

  sim::State no_burnet = route_state();
  erase_one(no_burnet.hand, sim::Card::ProfessorBurnet);
  unavailable(no_burnet, full_item_scenario(),
              "Missing Professor Burnet must block the payload route.");

  sim::State no_payload = route_state();
  erase_one(no_payload.deck, sim::Card::MegaDragonite);
  erase_one(no_payload.deck, sim::Card::Dragapult);
  unavailable(no_payload, full_item_scenario(),
              "Known payload collapse must block the route.");
}

void test_seed_12_regression() {
  const sim::Scenario chosen{"strict-jit-full-item-lock/go-second",
                             sim::DciProfile::StrictJit,
                             sim::LockMode::FullItem, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{12};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(chosen, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 4,
         "Seed 12 must improve from setup failure to readiness by T4.");
  expect(trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven must replace the redundant searched VSTAR with Grass Energy.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_blockers();
    test_seed_12_regression();
    std::cout << "Issue 1223 Steven Grass target tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

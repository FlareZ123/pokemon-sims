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

sim::Scenario no_lock_scenario(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1378", sim::DciProfile::StrictJit, locks, false, 5};
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
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(chosen, recipe, rng, nullptr);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_no_lock_route_and_boundaries() {
  std::mt19937_64 rng{137801};
  sim::Engine live = make_engine(no_lock_scenario(), rng, route_state());

  // Steven's Resolve searches any three cards. In this K1 state Regidrago VSTAR
  // and Professor Burnet are already held, so Regidrago V, Crispin, and Grass
  // Energy cover three complementary setup axes without duplicating VSTAR:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official search, attachment, evolution, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Existing full-Item-lock precedent: https://github.com/FlareZ123/pokemon-sims/issues/1223
  // Confirmed no-lock bug: https://github.com/FlareZ123/pokemon-sims/issues/1378
  expect(sim::EngineTestAccess::route_available(live),
         "The proven no-lock Steven Grass route must be available.");

  const auto unavailable = [&rng](sim::State state, const sim::Scenario scenario,
                                  const bool deck_seen, const char* message) {
    sim::Engine engine = make_engine(scenario, rng, std::move(state), deck_seen);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  unavailable(route_state(), no_lock_scenario(), false,
              "K0 must not use the prize-aware route projection.");

  sim::State no_vstar = route_state();
  erase_one(no_vstar.hand, sim::Card::RegidragoVstar);
  unavailable(no_vstar, no_lock_scenario(), true,
              "A missing held VSTAR must preserve direct VSTAR search.");

  sim::State held_grass = route_state();
  held_grass.hand.push_back(sim::Card::Grass);
  unavailable(held_grass, no_lock_scenario(), true,
              "A held Grass must preserve the existing target policy.");

  sim::State one_grass = route_state();
  erase_one(one_grass.deck, sim::Card::Grass);
  unavailable(one_grass, no_lock_scenario(), true,
              "Two deck Grass are required for Steven plus Crispin.");

  sim::State no_fire = route_state();
  erase_one(no_fire.deck, sim::Card::Fire);
  unavailable(no_fire, no_lock_scenario(), true,
              "Crispin requires two different Basic Energy types.");

  sim::State full_bench = route_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Dipplin, 1});
  unavailable(full_bench, no_lock_scenario(), true,
              "The route needs Bench space for Regidrago V and Latias ex.");

  unavailable(route_state(), no_lock_scenario(sim::LockMode::FullRuleBoxAbility), true,
              "Rule Box Ability lock must block the Latias route.");
  unavailable(route_state(), no_lock_scenario(sim::LockMode::FullSupporter), true,
              "Supporter lock must block Steven's Resolve.");

  sim::State no_stone = route_state();
  erase_one(no_stone.deck, sim::Card::ForestSealStone);
  unavailable(no_stone, no_lock_scenario(), true,
              "Missing Forest Seal Stone must block the projected route.");

  sim::State no_latias = route_state();
  erase_one(no_latias.deck, sim::Card::LatiasEx);
  unavailable(no_latias, no_lock_scenario(), true,
              "Missing Latias ex must block the promotion route.");

  sim::State no_burnet = route_state();
  erase_one(no_burnet.hand, sim::Card::ProfessorBurnet);
  unavailable(no_burnet, no_lock_scenario(), true,
              "Missing Professor Burnet must block the payload route.");

  sim::State no_payload = route_state();
  erase_one(no_payload.deck, sim::Card::MegaDragonite);
  erase_one(no_payload.deck, sim::Card::Dragapult);
  unavailable(no_payload, no_lock_scenario(), true,
              "Known payload collapse must block the route.");
}

void test_exact_seed_12_no_lock_regression() {
  const sim::Scenario chosen{"strict-jit/go-second", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{12};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(chosen, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The one-target substitution must improve this exact public/K1 route from a
  // diagnostic T5 setup failure to readiness by T4 without reading future draws:
  // Confirmed bug and reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1378
  // Existing reviewed target substitution: https://github.com/FlareZ123/pokemon-sims/issues/1223
  // Future-card oracle boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  expect(outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 4,
         "No-lock seed 12 must improve from T5 failure to readiness by T4.");
  expect(trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven must replace the redundant searched VSTAR with Grass Energy.");
  expect(!trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Regidrago VSTAR"),
         "Steven must not search a second VSTAR when one is already held.");
}
}  // namespace

int main() {
  try {
    test_no_lock_route_and_boundaries();
    test_exact_seed_12_no_lock_regression();
    std::cout << "Issue 1378 no-lock Steven Grass tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

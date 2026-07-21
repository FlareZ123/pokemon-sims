#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool package_available(const Engine& engine) {
    return engine.issue_1223_full_item_lock_energy_package_available();
  }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_has(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::Scenario full_item_scenario() {
  return sim::Scenario{"issue-1223", sim::DciProfile::StrictJit,
                       sim::LockMode::FullItem, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::ProfessorBurnet,
                sim::Card::RegidragoVstar, sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::ForestSealStone, sim::Card::LatiasEx,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::RegidragoVstar};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void erase_all(std::vector<sim::Card>& cards, const sim::Card card) {
  cards.erase(std::remove(cards.begin(), cards.end(), card), cards.end());
}

void test_exact_k1_package_and_target_order() {
  const sim::Scenario scenario = full_item_scenario();
  std::mt19937_64 rng{122301};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, rng, route_state(), &trace);

  // Steven's unrestricted three-card search must spend each target on an unresolved
  // axis. The held VSTAR is already available, while Regidrago V, Crispin, and Grass
  // produce the legal permanent-Item-lock attachment schedule:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1223
  expect(sim::EngineTestAccess::package_available(engine),
         "The exact K1 full-Item-lock energy package must be available.");
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven's Resolve did not play in the exact route state.");
  expect(count_card(engine.state().hand, sim::Card::RegidragoV) == 1,
         "Steven must search Regidrago V.");
  expect(count_card(engine.state().hand, sim::Card::Crispin) == 1,
         "Steven must search Crispin.");
  expect(count_card(engine.state().hand, sim::Card::Grass) == 1,
         "Steven must search Grass Energy.");
  expect(count_card(engine.state().hand, sim::Card::RegidragoVstar) == 1,
         "Steven must not fetch a redundant second VSTAR.");
  expect(trace_has(trace, "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven trace did not preserve the approved target order.");
}

void test_package_boundaries() {
  const sim::Scenario live = full_item_scenario();
  const auto reject = [&](sim::State state, const sim::Scenario scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::package_available(engine), message);
  };

  sim::State no_vstar = route_state();
  erase_all(no_vstar.hand, sim::Card::RegidragoVstar);
  reject(std::move(no_vstar), live, 122302,
         "Missing held VSTAR must reject the package.");

  sim::State no_regi = route_state();
  erase_all(no_regi.deck, sim::Card::RegidragoV);
  reject(std::move(no_regi), live, 122303,
         "A prized or otherwise unavailable Regidrago V must reject the package.");

  sim::State no_crispin = route_state();
  erase_all(no_crispin.deck, sim::Card::Crispin);
  reject(std::move(no_crispin), live, 122304,
         "A prized or otherwise unavailable Crispin must reject the package.");

  sim::State no_grass = route_state();
  erase_all(no_grass.deck, sim::Card::Grass);
  reject(std::move(no_grass), live, 122305,
         "Missing searchable Grass must reject the package.");

  sim::State one_type = route_state();
  erase_all(one_type.deck, sim::Card::Fire);
  reject(std::move(one_type), live, 122306,
         "A one-type Crispin state must reject the package.");

  sim::State no_fss = route_state();
  erase_all(no_fss.deck, sim::Card::ForestSealStone);
  reject(std::move(no_fss), live, 122307,
         "Unavailable Forest Seal Stone must reject the package.");

  sim::State held_fss = route_state();
  erase_all(held_fss.deck, sim::Card::ForestSealStone);
  held_fss.hand.push_back(sim::Card::ForestSealStone);
  {
    std::mt19937_64 rng{122308};
    sim::Engine engine = make_engine(live, rng, std::move(held_fss));
    expect(sim::EngineTestAccess::package_available(engine),
           "A held Forest Seal Stone must satisfy its route axis.");
  }

  sim::State no_latias = route_state();
  erase_all(no_latias.deck, sim::Card::LatiasEx);
  reject(std::move(no_latias), live, 122309,
         "Unavailable Latias ex must reject the package.");

  sim::State no_burnet = route_state();
  erase_all(no_burnet.hand, sim::Card::ProfessorBurnet);
  reject(std::move(no_burnet), live, 122310,
         "Missing held Professor Burnet must reject the package.");

  sim::State no_payload = route_state();
  erase_all(no_payload.deck, sim::Card::MegaDragonite);
  erase_all(no_payload.deck, sim::Card::Dragapult);
  reject(std::move(no_payload), live, 122311,
         "Missing deck payload must reject the package.");

  sim::State tight_bench = route_state();
  tight_bench.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 0});
  tight_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 0});
  tight_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0});
  reject(std::move(tight_bench), live, 122312,
         "Fewer than two open Bench slots must reject Regidrago plus Latias.");

  const sim::Scenario combined{"issue-1223-combined", sim::DciProfile::StrictJit,
                               sim::LockMode::FullCombined, false, 5};
  reject(route_state(), combined, 122313,
         "Rule Box Ability lock must reject the Skyliner package.");
}

void test_seed_12_regression() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-full-item-lock/go-second");
  if (!scenario) throw std::runtime_error("Missing full-Item-lock scenario");

  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{12};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 4,
         "Seed 12 must reach the isolated #1223 floor on T4.");
  expect(trace_has(trace,
                   "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Seed 12 did not select the approved Steven package.");
  expect(trace_has(trace, "T2 | POLICY") && trace_has(trace, "Regidrago V [GG]"),
         "Seed 12 did not establish two Grass Energy on T2.");
  expect(trace_has(trace, "T4 | READY |"),
         "Seed 12 did not complete the isolated T4 route.");
}
}  // namespace

int main() {
  test_exact_k1_package_and_target_order();
  test_package_boundaries();
  test_seed_12_regression();
  return 0;
}

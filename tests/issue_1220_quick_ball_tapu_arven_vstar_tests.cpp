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
  static bool route_available(Engine& engine) {
    return engine.issue_1220_quick_ball_tapu_arven_vstar_route_available();
  }
  static bool run_search_step(Engine& engine) {
    return engine.run_search_items_one_step(false);
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
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

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1220", sim::DciProfile::NoDiscardControl, locks, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1}};
  state.hand = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::BrilliantBlender, sim::Card::QuickBall,
                sim::Card::QuickBall, sim::Card::Gladion};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Arven,
                sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Oricorio};
  state.discard = {sim::Card::GoodraVstar, sim::Card::Crispin};
  return state;
}

sim::Engine make_engine(const sim::Scenario& chosen, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(chosen, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_exact_route_completes_turn_four() {
  std::mt19937_64 rng{122001};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario(), rng, route_state(), &trace);

  // The complete K1 graph must remain payable before Quick Ball is spent:
  // Quick Ball -> Tapu Lele-GX -> Arven -> Mysterious Treasure -> Regidrago VSTAR.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1220
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact K1 Quick Ball-Tapu-Arven-VSTAR bridge must be available.");
  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "The prior-turn GGF Regidrago must evolve and become Active.");
  expect(after.active->grass >= 2 && after.active->fire >= 1,
         "The promoted VSTAR must retain GGF.");
  expect(count_card(after.discard, sim::Card::QuickBall) == 1,
         "Exactly one Quick Ball must be played.");
  expect(count_card(after.discard, sim::Card::MegaDragonite) == 1,
         "The first redundant Dragon must pay Quick Ball.");
  expect(count_card(after.discard, sim::Card::Dragapult) == 1,
         "The second redundant Dragon must pay Mysterious Treasure.");
  expect(trace_contains(trace, "Searched a Basic Pokémon: Tapu Lele-GX"),
         "Quick Ball must search Tapu Lele-GX.");
  expect(trace_contains(trace, "WONDER TAG"),
         "Tapu Lele-GX must resolve Wonder Tag.");
  expect(trace_contains(trace, "Searched and revealed Arven"),
         "Wonder Tag must search Arven.");
  expect(trace_contains(trace, "Searched a Psychic or Dragon Pokémon: Regidrago VSTAR"),
         "Mysterious Treasure must search Regidrago VSTAR.");
  expect(trace_contains(trace, "RETREAT"),
         "Skyliner must provide the free promotion.");
}

void test_route_preflight_blocks_missing_or_illegal_edges() {
  const auto blocked = [](sim::State state, const sim::Scenario chosen,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(chosen, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  sim::State no_tapu = route_state();
  erase_one(no_tapu.deck, sim::Card::TapuLeleGX);
  blocked(no_tapu, scenario(), 122002, "Known missing Tapu must block the bridge.");

  sim::State no_arven = route_state();
  erase_one(no_arven.deck, sim::Card::Arven);
  blocked(no_arven, scenario(), 122003, "Known missing Arven must block the bridge.");

  sim::State no_item = route_state();
  erase_one(no_item.deck, sim::Card::MysteriousTreasure);
  blocked(no_item, scenario(), 122004, "Missing Arven VSTAR Item must block the bridge.");

  sim::State no_vstar = route_state();
  erase_one(no_vstar.deck, sim::Card::RegidragoVstar);
  blocked(no_vstar, scenario(), 122005, "Known missing VSTAR must block the bridge.");

  sim::State one_cost = route_state();
  erase_one(one_cost.hand, sim::Card::Dragapult);
  blocked(one_cost, scenario(), 122006, "One protected remaining hand must not fund two costs.");

  sim::State new_regi = route_state();
  new_regi.bench.front().entered_turn = 4;
  blocked(new_regi, scenario(), 122007, "A same-turn Regidrago V cannot evolve.");

  sim::State full_bench = route_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  blocked(full_bench, scenario(), 122008, "A full Bench must block Tapu Lele-GX.");

  sim::State wrong_active = route_state();
  wrong_active.active = sim::Pokemon{sim::Card::Oricorio, 1};
  blocked(wrong_active, scenario(), 122009, "No Latias promotion route must block the bridge.");

  blocked(route_state(), scenario(sim::LockMode::FullItem), 122010,
          "Item lock must block Quick Ball and the Arven Item.");
  blocked(route_state(), scenario(sim::LockMode::FullRuleBoxAbility), 122011,
          "Rule Box Ability lock must block Wonder Tag and Skyliner.");
}

void test_direct_vstar_item_stays_preferred() {
  std::mt19937_64 rng{122012};
  sim::State state = route_state();
  state.hand.push_back(sim::Card::MysteriousTreasure);
  sim::Engine engine = make_engine(scenario(), rng, std::move(state));

  expect(sim::EngineTestAccess::run_search_step(engine),
         "A direct held VSTAR Item must execute.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_card(after.hand, sim::Card::QuickBall) == 2,
         "The indirect Quick Ball bridge must remain untouched when Treasure is held.");
  expect(count_card(after.hand, sim::Card::RegidragoVstar) == 1,
         "The direct Mysterious Treasure route must search VSTAR.");
}

void test_seed_10_regression() {
  const sim::Scenario chosen{"no-discard-control/go-second",
                             sim::DciProfile::NoDiscardControl,
                             sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{10};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(chosen, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 4,
         "Seed 10 must complete the legal bridge on T4.");
  expect(trace_contains(trace, "T4 | PLAY ITEM | rules: R-QB-01"),
         "Seed 10 must play Quick Ball on T4.");
  expect(trace_contains(trace, "T4 | WONDER TAG"),
         "Seed 10 must use Wonder Tag on T4.");
  expect(trace_contains(trace, "T4 | PLAY SUPPORTER | rules: R-ARVEN-01"),
         "Seed 10 must play Arven on T4.");
  expect(trace_contains(trace, "T4 | READY |"),
         "Seed 10 must record T4 readiness.");
}
}  // namespace

int main() {
  try {
    test_exact_route_completes_turn_four();
    test_route_preflight_blocks_missing_or_illegal_edges();
    test_direct_vstar_item_stays_preferred();
    test_seed_10_regression();
    std::cout << "Issue 1220 Quick Ball-Tapu-Arven-VSTAR tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

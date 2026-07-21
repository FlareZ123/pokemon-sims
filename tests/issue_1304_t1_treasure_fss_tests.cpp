#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool route_available(Engine& engine) {
    return engine.issue_1304_t1_treasure_erika_cost_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static void play_basics(Engine& engine) { engine.play_basics_from_hand(); }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool hold_fss(Engine& engine) {
    return engine.hold_fss_for_next_turn_secret_box();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::None,
                       const int max_turn = 5,
                       const bool going_first = true) {
  return sim::Scenario{"issue-1304", sim::DciProfile::StrictJit,
                       locks, going_first, max_turn};
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Pineco, 0};
  state.hand = {sim::Card::GoodraVstar, sim::Card::ForestOfVitality,
                sim::Card::ForretressEx, sim::Card::Gladion,
                sim::Card::MysteriousTreasure, sim::Card::ErikasInvitation,
                sim::Card::ForestSealStone};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure, sim::Card::Fire,
                sim::Card::Grass, sim::Card::SecretBox,
                sim::Card::TapuLeleGX};
  return state;
}

sim::Engine make_engine(const sim::Scenario& chosen,
                        std::mt19937_64& rng,
                        sim::State state,
                        const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(chosen, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_exact_t1_route_spends_erika_and_holds_star_alchemy() {
  std::mt19937_64 rng{130401};
  sim::Engine engine = make_engine(scenario(), rng, route_state());

  // Mysterious Treasure may discard Erika's Invitation and search Regidrago V.
  // Erika is setup-dead in the repository's opponent-free model, while this search
  // establishes the prior-turn Basic required by the complete T2 route:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1304
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact public T1 Treasure-FSS route must be admitted.");
  expect(sim::EngineTestAccess::play_treasure(engine),
         "Mysterious Treasure must resolve with Erika as its cost.");
  sim::EngineTestAccess::play_basics(engine);
  expect(sim::EngineTestAccess::attach_fss(engine),
         "Forest Seal Stone must attach to the searched Regidrago V.");
  expect(sim::EngineTestAccess::hold_fss(engine),
         "Star Alchemy must be held as the next-turn Fire channel.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MysteriousTreasure),
         "The played Treasure must enter discard.");
  expect(contains(after.discard, sim::Card::ErikasInvitation),
         "Erika's Invitation must pay the Treasure cost.");
  expect(sim::EngineTestAccess::state(engine).bench.size() == 1U &&
             sim::EngineTestAccess::state(engine).bench.front().card == sim::Card::RegidragoV,
         "The searched Regidrago V must be Benched on T1.");
  expect(!after.vstar_power_used,
         "The T1 route must preserve Star Alchemy for T2.");
}

void expect_route_blocked(sim::State state,
                          const sim::Scenario chosen,
                          const std::uint64_t seed,
                          const char* message) {
  std::mt19937_64 rng{seed};
  sim::Engine engine = make_engine(chosen, rng, std::move(state));
  expect(!sim::EngineTestAccess::route_available(engine), message);
}

void test_route_controls() {
  sim::State no_cost = route_state();
  erase_one(no_cost.hand, sim::Card::ErikasInvitation);
  expect_route_blocked(no_cost, scenario(), 130402,
                       "No payable Erika cost must block the exception.");

  sim::State no_regi = route_state();
  erase_one(no_regi.deck, sim::Card::RegidragoV);
  expect_route_blocked(no_regi, scenario(), 130403,
                       "Known missing Regidrago V must block the route.");

  sim::State no_fss = route_state();
  erase_one(no_fss.hand, sim::Card::ForestSealStone);
  expect_route_blocked(no_fss, scenario(), 130404,
                       "Missing Forest Seal Stone must block the route.");

  sim::State no_forretress = route_state();
  erase_one(no_forretress.hand, sim::Card::ForretressEx);
  expect_route_blocked(no_forretress, scenario(), 130405,
                       "Missing Forretress ex must block the route.");

  sim::State no_vstar = route_state();
  erase_one(no_vstar.deck, sim::Card::RegidragoVstar);
  expect_route_blocked(no_vstar, scenario(), 130406,
                       "Known missing Regidrago VSTAR must block the route.");

  sim::State no_fire = route_state();
  erase_one(no_fire.deck, sim::Card::Fire);
  expect_route_blocked(no_fire, scenario(), 130407,
                       "Known missing Fire must block the route.");

  sim::State no_grass = route_state();
  erase_one(no_grass.deck, sim::Card::Grass);
  expect_route_blocked(no_grass, scenario(), 130408,
                       "Known missing Grass must block Exploding Energy.");

  expect_route_blocked(route_state(), scenario(sim::LockMode::FullItem), 130409,
                       "Item lock must block Mysterious Treasure.");
  expect_route_blocked(route_state(), scenario(sim::LockMode::FullRuleBoxAbility), 130410,
                       "Rule Box Ability lock must block Exploding Energy.");
  expect_route_blocked(route_state(), scenario(sim::LockMode::None, 1), 130411,
                       "No remaining T2 horizon must block the route.");
  expect_route_blocked(route_state(), scenario(sim::LockMode::None, 5, false), 130412,
                       "The going-first-specific projection must not alter going-second policy.");
}

void test_seed_31415_reaches_strict_jit_on_t2() {
  const sim::Scenario chosen{"strict-jit/go-first", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  std::mt19937_64 rng{31415};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(chosen, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The paid T1 Treasure shuffle deterministically yields the second Treasure on
  // T2 for this source-bound seed. Exploding Energy supplies Grass, Star Alchemy
  // searches Fire, and the drawn Treasure discards Goodra while finding VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1304
  expect(outcome.first_ready_turn == 2,
         "Pineco seed 31415 must reach strict-JIT readiness on T2.");
  expect(trace_contains(trace, "T1 | DISCARD | rules: R-MT-01 | Erika's Invitation"),
         "T1 Treasure must discard Erika's Invitation.");
  expect(trace_contains(trace, "T1 | HOLD VSTAR POWER"),
         "T1 must preserve Star Alchemy.");
  expect(trace_contains(trace, "T2 | DRAW | rules: R-GAME-DRAW | start-of-turn: Mysterious Treasure"),
         "The source-bound paid shuffle must produce the T2 Treasure draw.");
  expect(trace_contains(trace, "T2 | USE VSTAR POWER | rules: R-FSS-01; R-GAME-ENERGY"),
         "Star Alchemy must search Fire on T2.");
  expect(trace_contains(trace, "T2 | READY |"),
         "The trace must record strict-JIT readiness on T2.");
}
}  // namespace

int main() {
  try {
    test_exact_t1_route_spends_erika_and_holds_star_alchemy();
    test_route_controls();
    test_seed_31415_reaches_strict_jit_on_t2();
    std::cout << "Issue 1304 T1 Treasure-FSS tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

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
  static bool t1_route_available(const Engine& engine) {
    return engine.issue_1209_t1_treasure_tapu_crispin_route_available();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool t2_completion_available(const Engine& engine) {
    return engine.issue_1209_t2_treasure_tapu_crispin_completion_available();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
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

sim::Scenario strict_first(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1209", sim::DciProfile::StrictJit, locks, true, 5};
}

sim::State t1_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass, sim::Card::Grass,
                sim::Card::MysteriousTreasure, sim::Card::Dragapult,
                sim::Card::RegidragoVstar, sim::Card::Arven};
  state.deck = {sim::Card::RegidragoV, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

sim::State t2_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dragapult,
                sim::Card::RegidragoVstar, sim::Card::Arven};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::MegaDragonite};
  state.manual_energy_used = true;
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr,
                        const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_t1_dynamic_grass_cost_and_exact_search() {
  std::mt19937_64 rng{120901};
  const sim::Scenario scenario = strict_first();
  sim::Engine engine = make_engine(scenario, rng, t1_route_state());

  // Quick Ball may spend one of two held Grass only because the observable graph
  // preserves the second Grass for T1 and proves the K1 Treasure -> Tapu -> Crispin
  // T2 continuation. Every connector and cost is source-bounded in the test state:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(sim::EngineTestAccess::t1_route_available(engine),
         "The exact T1 Treasure-Tapu-Crispin continuation must admit one Grass cost.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must execute the exact route-conditioned T1 search.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_card(after.discard, sim::Card::Grass) == 1,
         "Exactly one Grass must pay Quick Ball.");
  expect(count_card(after.hand, sim::Card::Grass) == 1,
         "The second Grass must remain for the T1 manual attachment.");
  expect(count_card(after.hand, sim::Card::RegidragoV) == 1,
         "Quick Ball must search Regidrago V.");
}

void test_lower_dci_cost_stays_ahead_of_grass() {
  std::mt19937_64 rng{120926};
  const sim::Scenario scenario = strict_first();
  sim::State state = t1_route_state();
  state.hand.push_back(sim::Card::QuickBall);
  sim::Engine engine = make_engine(scenario, rng, std::move(state));

  // A duplicate Quick Ball is ordinary lower-DCI fuel and must remain ahead of the
  // route-conditioned Grass exception:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "The duplicate-Item Quick Ball route must remain payable.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_card(after.discard, sim::Card::QuickBall) == 2,
         "The played copy and duplicate cost must enter discard.");
  expect(count_card(after.discard, sim::Card::Grass) == 0,
         "Grass must remain protected while lower-DCI fuel exists.");
}

void test_t1_missing_route_parts_protect_grass() {
  const auto blocked = [](sim::State state, const sim::Scenario scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::t1_route_available(engine), message);
    expect(!sim::EngineTestAccess::play_quick_ball(engine), message);
  };

  sim::State one_grass = t1_route_state();
  erase_one(one_grass.hand, sim::Card::Grass);
  blocked(one_grass, strict_first(), 120902,
          "The only held Grass must remain protected.");

  sim::State no_treasure = t1_route_state();
  erase_one(no_treasure.hand, sim::Card::MysteriousTreasure);
  blocked(no_treasure, strict_first(), 120903,
          "Missing Treasure must block the projected connector.");

  sim::State no_payload = t1_route_state();
  erase_one(no_payload.hand, sim::Card::Dragapult);
  blocked(no_payload, strict_first(), 120904,
          "Missing current-turn Dragon payload must block the route.");

  sim::State no_vstar = t1_route_state();
  erase_one(no_vstar.hand, sim::Card::RegidragoVstar);
  blocked(no_vstar, strict_first(), 120905,
          "Missing held VSTAR must block the T2 evolution route.");

  sim::State no_regi = t1_route_state();
  erase_one(no_regi.deck, sim::Card::RegidragoV);
  blocked(no_regi, strict_first(), 120906,
          "Known missing Regidrago V must block the exact route.");

  sim::State no_tapu = t1_route_state();
  erase_one(no_tapu.deck, sim::Card::TapuLeleGX);
  blocked(no_tapu, strict_first(), 120907,
          "Known missing Tapu Lele-GX must block the exact route.");

  sim::State no_crispin = t1_route_state();
  erase_one(no_crispin.deck, sim::Card::Crispin);
  blocked(no_crispin, strict_first(), 120908,
          "Known missing Crispin must block the exact route.");

  sim::State no_fire = t1_route_state();
  erase_one(no_fire.deck, sim::Card::Fire);
  blocked(no_fire, strict_first(), 120909,
          "Known missing Fire must block the exact route.");

  sim::State full_bench = t1_route_state();
  full_bench.bench = {sim::Pokemon{sim::Card::RegidragoV, 0},
                      sim::Pokemon{sim::Card::Oricorio, 0},
                      sim::Pokemon{sim::Card::TapuLeleGX, 0},
                      sim::Pokemon{sim::Card::DialgaGX, 0}};
  blocked(full_bench, strict_first(), 120910,
          "Fewer than two open Bench slots must block the route.");

  sim::State wrong_active = t1_route_state();
  wrong_active.active = sim::Pokemon{sim::Card::Oricorio, 0};
  blocked(wrong_active, strict_first(), 120911,
          "A non-Skyliner Active must block the exact promotion route.");

  blocked(t1_route_state(),
          sim::Scenario{"issue-1209-lock", sim::DciProfile::StrictJit,
                        sim::LockMode::TurnTwoItem, true, 5},
          120912, "Scheduled T2 Item lock must block the Treasure continuation.");

  blocked(t1_route_state(),
          sim::Scenario{"issue-1209-ability-lock", sim::DciProfile::StrictJit,
                        sim::LockMode::FullRuleBoxAbility, true, 5},
          120913, "Rule Box Ability lock must block Wonder Tag and Skyliner.");
}

void test_t2_holds_arven_and_completes_route() {
  std::mt19937_64 rng{120914};
  const sim::Scenario scenario = strict_first();
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, rng, t2_route_state(), &trace);

  expect(sim::EngineTestAccess::t2_completion_available(engine),
         "The exact K1 T2 route must be recognized as a complete finish.");
  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);

  // The Supporter slot stays open through the paid Treasure search, then Wonder Tag
  // obtains Crispin. Crispin attaches Fire, the prior-turn Basic evolves, and Skyliner
  // promotes the GGF VSTAR with Dragapult discarded during the same turn:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "The completed Regidrago VSTAR must be Active.");
  expect(after.active->grass >= 2 && after.active->fire >= 1,
         "The Active VSTAR must reach GGF.");
  expect(count_card(after.discard, sim::Card::Dragapult) == 1,
         "The Dragon payload must enter discard during T2.");
  expect(count_card(after.hand, sim::Card::Arven) == 1,
         "Arven must remain held instead of consuming the Supporter play.");
  expect(count_card(after.discard, sim::Card::Crispin) == 1,
         "Wonder Tag's Crispin must be the Supporter played.");
  expect(trace_contains(trace, "HOLD SUPPORTER"),
         "The trace must record the route-preserving Supporter hold.");
  expect(trace_contains(trace, "searched Tapu Lele-GX for the Crispin completion route"),
         "The trace must record the source-bounded Treasure target.");
}

void test_t2_completion_does_not_require_a_drawn_energy() {
  std::mt19937_64 rng{120927};
  const sim::Scenario scenario = strict_first();
  sim::State state = t2_route_state();
  state.bench.front().grass = 1;
  state.manual_energy_used = false;
  sim::Engine engine = make_engine(scenario, rng, std::move(state));

  // With only the T1 Grass attached, Crispin still searches both Basic Energy types,
  // attaches one, and places the other in hand for the unused manual attachment:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(sim::EngineTestAccess::t2_completion_available(engine),
         "The T2 route must not depend on drawing a Basic Energy.");
  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass >= 2 && after.active->fire >= 1,
         "Crispin plus the manual attachment must complete GGF without a drawn Energy.");
  expect(count_card(after.discard, sim::Card::Dragapult) == 1,
         "The same-turn Dragon payload must still enter discard.");
}

void test_t2_controls_leave_arven_policy_live() {
  const auto unavailable = [](sim::State state, const sim::Scenario scenario,
                              const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::t2_completion_available(engine), message);
  };

  sim::State no_tapu = t2_route_state();
  erase_one(no_tapu.deck, sim::Card::TapuLeleGX);
  unavailable(no_tapu, strict_first(), 120915,
              "Known missing Tapu must block the T2 hold.");

  sim::State no_crispin = t2_route_state();
  erase_one(no_crispin.deck, sim::Card::Crispin);
  unavailable(no_crispin, strict_first(), 120916,
              "Known missing Crispin must block the T2 hold.");

  sim::State no_grass = t2_route_state();
  erase_one(no_grass.deck, sim::Card::Grass);
  unavailable(no_grass, strict_first(), 120917,
              "Known missing Grass must block the two-type Crispin route.");

  sim::State no_fire = t2_route_state();
  erase_one(no_fire.deck, sim::Card::Fire);
  unavailable(no_fire, strict_first(), 120918,
              "Known missing Fire must block the T2 finish.");

  sim::State no_vstar = t2_route_state();
  erase_one(no_vstar.hand, sim::Card::RegidragoVstar);
  unavailable(no_vstar, strict_first(), 120919,
              "Missing VSTAR must block the evolution route.");

  sim::State no_payload = t2_route_state();
  erase_one(no_payload.hand, sim::Card::Dragapult);
  unavailable(no_payload, strict_first(), 120920,
              "Missing payload must block the Treasure route.");

  sim::State new_regi = t2_route_state();
  new_regi.bench.front().entered_turn = 2;
  unavailable(new_regi, strict_first(), 120921,
              "A same-turn Regidrago V cannot satisfy the evolution window.");

  sim::State wrong_active = t2_route_state();
  wrong_active.active = sim::Pokemon{sim::Card::Oricorio, 0};
  unavailable(wrong_active, strict_first(), 120922,
              "A non-Latias Active must block the exact promotion route.");

  unavailable(t2_route_state(),
              sim::Scenario{"issue-1209-item-lock", sim::DciProfile::StrictJit,
                            sim::LockMode::TurnTwoItem, true, 5},
              120923, "T2 Item lock must block Mysterious Treasure.");

  unavailable(t2_route_state(),
              sim::Scenario{"issue-1209-rulebox-lock", sim::DciProfile::StrictJit,
                            sim::LockMode::FullRuleBoxAbility, true, 5},
              120924, "Rule Box Ability lock must block Wonder Tag and Skyliner.");

  std::mt19937_64 rng{120925};
  const sim::Scenario scenario = strict_first();
  sim::State missing_fire = t2_route_state();
  erase_one(missing_fire.deck, sim::Card::Fire);
  missing_fire.deck.push_back(sim::Card::QuickBall);
  missing_fire.deck.push_back(sim::Card::ForestSealStone);
  sim::Engine engine = make_engine(scenario, rng, std::move(missing_fire));
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used && count_card(after.discard, sim::Card::Arven) == 1,
         "Arven must remain available when the complete Treasure route is absent.");
}

void test_seed_43_reaches_turn_two_without_draw_dependency() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{43};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 2,
         "Seed 43 must become ready on turn two.");
  expect(trace_contains(trace, "T1 | DISCARD | rules: R-QB-01 | Grass Energy"),
         "Seed 43 must use the dynamic T1 Grass cost.");
  expect(trace_contains(trace, "T2 | HOLD SUPPORTER"),
         "Seed 43 must preserve the Supporter slot before Treasure.");
  expect(trace_contains(trace, "T2 | WONDER TAG | rules: R-TAPU-01"),
         "Seed 43 must resolve Wonder Tag on turn two.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-CRISPIN-01"),
         "Seed 43 must play Crispin on turn two.");
  expect(trace_contains(trace, "T2 | READY |"),
         "Seed 43 must record turn-two readiness.");
}
}  // namespace

int main() {
  try {
    test_t1_dynamic_grass_cost_and_exact_search();
    test_lower_dci_cost_stays_ahead_of_grass();
    test_t1_missing_route_parts_protect_grass();
    test_t2_holds_arven_and_completes_route();
    test_t2_completion_does_not_require_a_drawn_energy();
    test_t2_controls_leave_arven_policy_live();
    test_seed_43_reaches_turn_two_without_draw_dependency();
    std::cout << "Issue 1209 Treasure-Tapu-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_deck_seen(Engine& engine, const bool seen) {
    engine.deck_seen_ = seen;
  }
  static bool route_available(Engine& engine) {
    return engine.issue_1411_t1_treasure_tapu_steven_route_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

struct Fixture {
  sim::Scenario scenario{"issue-1411/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 2};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1411};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State complete_t1_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Fire,
                sim::Card::Powerglass,
                sim::Card::Lusamine,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::FieldBlower,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::TapuLeleGX,
                sim::Card::StevensResolve,
                sim::Card::RegidragoVstar,
                sim::Card::Crispin,
                sim::Card::BrilliantBlender,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::Dragapult};
  return state;
}

void test_complete_route_is_admitted() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, complete_t1_route_state());

  // Field Blower has zero present AMR in the no-lock setup state. Paying it to
  // Mysterious Treasure exposes K1 and unlocks Tapu -> Steven -> VSTAR, Crispin,
  // Blender, which completes the legal T2 GGF plus strict-JIT payload route:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1411
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The complete T1 Treasure-Tapu-Steven route was not admitted.");
}

void test_incomplete_routes_preserve_field_blower() {
  const auto expect_blocked = [](sim::State state, const char* message,
                                 const bool deck_seen = false) {
    Fixture fixture;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(fixture.engine, deck_seen);
    expect(!sim::EngineTestAccess::route_available(fixture.engine), message);
  };

  sim::State one_grass = complete_t1_route_state();
  one_grass.hand.erase(std::find(one_grass.hand.begin(), one_grass.hand.end(),
                                 sim::Card::Grass));
  expect_blocked(std::move(one_grass),
                 "Field Blower became discardable with only one held Grass.");

  sim::State full_bench = complete_t1_route_state();
  for (int index = 0; index < 5; ++index) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                                            sim::Tool::None});
  }
  expect_blocked(std::move(full_bench),
                 "Field Blower became discardable without a Tapu Bench slot.");

  sim::State no_tapu = complete_t1_route_state();
  no_tapu.deck.erase(std::find(no_tapu.deck.begin(), no_tapu.deck.end(),
                               sim::Card::TapuLeleGX));
  expect_blocked(std::move(no_tapu),
                 "A K1 state without Tapu still admitted the route.", true);

  sim::State no_steven = complete_t1_route_state();
  no_steven.deck.erase(std::find(no_steven.deck.begin(), no_steven.deck.end(),
                                 sim::Card::StevensResolve));
  expect_blocked(std::move(no_steven),
                 "A K1 state without Steven still admitted the route.", true);

  sim::State no_blender = complete_t1_route_state();
  no_blender.deck.erase(std::find(no_blender.deck.begin(), no_blender.deck.end(),
                                  sim::Card::BrilliantBlender));
  expect_blocked(std::move(no_blender),
                 "A K1 state without Brilliant Blender still admitted the route.", true);

  sim::State no_fire = complete_t1_route_state();
  no_fire.deck.erase(std::find(no_fire.deck.begin(), no_fire.deck.end(),
                               sim::Card::Fire));
  expect_blocked(std::move(no_fire),
                 "A K1 state without searchable Fire still admitted the route.", true);
}

void test_lock_scenarios_preserve_field_blower() {
  const auto state = complete_t1_route_state();

  sim::Scenario ability_lock{"issue-1411/ability-lock",
                             sim::DciProfile::StrictJit,
                             sim::LockMode::FullRuleBoxAbility, false, 2};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng1{14111};
  sim::Engine ability_engine{ability_lock, recipe, rng1};
  sim::EngineTestAccess::set_state(ability_engine, state);
  expect(!sim::EngineTestAccess::route_available(ability_engine),
         "Field Blower was discarded while it remained the Rule Box lock answer.");

  sim::Scenario item_lock{"issue-1411/item-lock",
                          sim::DciProfile::StrictJit,
                          sim::LockMode::TurnTwoItem, false, 2};
  std::mt19937_64 rng2{14112};
  sim::Engine item_engine{item_lock, recipe, rng2};
  sim::EngineTestAccess::set_state(item_engine, state);
  expect(!sim::EngineTestAccess::route_available(item_engine),
         "The T1 route was admitted when T2 Item lock blocks Blender.");
}

void test_seed_144_reaches_t2_through_tapu_steven() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1411 seed fixture is unavailable.");

  std::mt19937_64 rng(144);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact source-bound regression. The corrected line spends Field Blower only
  // after proving the no-lock T1 compression route and reaches T2 without either
  // empty Celestial Roar:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1411
  expect(outcome.first_ready_turn == 2,
         "Seed 144 did not improve from T3 to T2.");
  expect(trace_contains(trace, "setup-dead Field Blower"),
         "Seed 144 did not use Field Blower as the dynamic-DCI Treasure cost.");
  expect(trace_contains(trace, "Tapu Lele-GX"),
         "Seed 144 did not search and Bench Tapu Lele-GX.");
  expect(trace_contains(trace, "Steven's Resolve"),
         "Seed 144 did not resolve the T1 Steven compression line.");
  expect(trace_contains(trace, "T2 | READY"),
         "Seed 144 trace no longer records T2 readiness.");
  expect(!trace_contains(trace, "Celestial Roar"),
         "Seed 144 still used the slower Celestial Roar route.");
}

}  // namespace

int main() {
  try {
    test_complete_route_is_admitted();
    test_incomplete_routes_preserve_field_blower();
    test_lock_scenarios_preserve_field_blower();
    test_seed_144_reaches_t2_through_tapu_steven();
    std::cout << "Issue 1411 Treasure-Tapu-Steven tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

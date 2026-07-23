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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.late_steven_active_vstar_crispin_treasure_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Crispin, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::RegidragoV,
                sim::Card::LatiasEx, sim::Card::FieldBlower};
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::LockMode lock, const std::uint64_t seed)
      : scenario{"issue-1425-lock-boundary", sim::DciProfile::StrictJit,
                 lock, true, 3},
        recipe{sim::baseline_recipe()},
        rng{seed},
        engine{scenario, recipe, rng} {
    sim::EngineTestAccess::set_state(engine, route_state());
  }
};

void test_rule_box_lock_admits_non_ability_route() {
  Fixture fixture{sim::LockMode::FullRuleBoxAbility, 142501};

  // Path to the Peak removes Abilities from Rule Box Pokémon. It does not block
  // Supporters, Items, evolution, or Energy attachments. The continuation uses no
  // Rule Box Ability, so the already proven Steven plus Crispin plus Treasure line
  // remains legal:
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1425
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "Rule Box Ability lock incorrectly rejected the non-Ability route.");
}

void test_item_locks_still_reject_treasure_continuation() {
  for (const sim::LockMode lock : {sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullItem,
                                   sim::LockMode::FullCombined}) {
    Fixture fixture{lock, 142502U + static_cast<unsigned>(lock)};

    // Mysterious Treasure is an Item. Item lock and combined lock make the planned
    // next-turn continuation illegal even though Steven and Crispin are Supporters:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Repository Item-lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
    // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1425
    expect(!sim::EngineTestAccess::route_available(fixture.engine),
           "An Item-lock scenario admitted the Treasure continuation.");
  }
}

void test_seed44_chooses_steven_under_rule_box_lock() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-rulebox-ability-lock/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The source-bound seed-44 scenario is unavailable.");

  std::mt19937_64 rng{44};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact public K1 state must reuse issue #1098's 41/42 route under the
  // repository's Path-to-the-Peak lock model instead of falling through to Serena:
  // Existing route proof: https://github.com/FlareZ123/pokemon-sims/issues/1098
  // Rule Box lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // Action priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1425
  expect(outcome.first_ready_turn == 3,
         "Seed 44 lost its legal T3 Rule Box-lock readiness.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(trace, "Searched the late Crispin-Treasure route"),
         "Seed 44 did not choose the legal Steven continuation.");
  expect(!trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-SERENA-01"),
         "Seed 44 still used Serena's weaker refresh.");
  expect(trace_contains(trace, "T3 | READY |"),
         "Seed 44 did not record T3 readiness.");
}
}  // namespace

int main() {
  try {
    test_rule_box_lock_admits_non_ability_route();
    test_item_locks_still_reject_treasure_continuation();
    test_seed44_chooses_steven_under_rule_box_lock();
    std::cout << "Issue 1425 Rule Box late-Steven tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

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
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }

  static bool entry_state(const Engine& engine) {
    return engine.issue_1704_t1_public_entry_state();
  }

  static bool play_route(Engine& engine) {
    return engine.play_issue_1704_t1_item_lock_burnet_route();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State exact_t1_state(const bool include_cost = true,
                          const bool include_vstar = true) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::Pineco, 0, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::TapuLeleGX, sim::Card::EarthenVessel,
                sim::Card::StevensResolve};
  if (include_vstar) state.hand.push_back(sim::Card::RegidragoVstar);
  if (include_cost) state.hand.push_back(sim::Card::ForestOfVitality);
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::ForretressEx,
                sim::Card::Fire, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Dragapult};
  state.prizes = {sim::Card::SecretBox};
  return state;
}

void test_registered_seed_reaches_t2_through_burnet() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-turn2-item-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1704 registered setup is unavailable.");

  std::mt19937_64 rng{1};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // T1 Wonder Tag may search Burnet after legal K1 inspection. Vessel then searches
  // Fire, Steven banks only Forretress ex, and T2 Burnet supplies the strict-JIT
  // payload after Exploding Energy supplies GG:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, Supporter, attachment, evolution, Ability, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Scheduled-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1704
  expect(outcome.first_ready_turn == 2,
         "Pineco seed 1 must reach the earliest legal T2 ready state.");
  expect(trace_contains(trace, "Wonder Tag searched Professor Burnet"),
         "Wonder Tag must preserve Burnet for the locked T2 payload bridge.");
  expect(trace_contains(trace, "Earthen Vessel discarded") &&
             trace_contains(trace, "searched Fire Energy before the scheduled Item lock"),
         "T1 Vessel must resolve before the scheduled Item lock.");
  expect(trace_contains(trace, "searched only Forretress ex"),
         "Steven must bank the Stage 1 instead of locked Items.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-BURNET-01") &&
             trace_contains(trace, "T2 | READY"),
         "T2 Burnet must establish the current-turn payload and readiness.");
  expect(!trace_contains(trace, "T1 | PLAY ITEM | rules: R-SECRET-BOX-01"),
         "The known locked next turn must not bank or spend Secret Box.");
}

void test_entry_requires_realistic_vessel_cost() {
  const sim::Scenario scenario{"issue-1704-cost-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 2};
  const auto recipe = sim::pineco_recipe();
  std::mt19937_64 rng{17040};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_t1_state(false, true));

  // Earthen Vessel requires another card as its discard cost:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // Active-move realism and DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1704
  expect(!sim::EngineTestAccess::entry_state(engine),
         "The override must reject a hand without its separate Vessel cost.");
  expect(!sim::EngineTestAccess::play_route(engine),
         "The route must not mutate a cost-blocked state.");
}

void test_entry_requires_held_vstar() {
  const sim::Scenario scenario{"issue-1704-vstar-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 2};
  const auto recipe = sim::pineco_recipe();
  std::mt19937_64 rng{17041};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_t1_state(true, false));

  // Regidrago VSTAR must already be held because T2 Items are unavailable:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // Scheduled Item-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1704
  expect(!sim::EngineTestAccess::entry_state(engine),
         "The override must reject a missing held VSTAR axis.");
}

void test_other_lock_profiles_keep_existing_policy() {
  for (const std::string label : {"strict-jit/go-second",
                                  "strict-jit-turn2-item-lock/go-first"}) {
    const auto scenario = sim::scenario_by_label(label);
    const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
    expect(scenario.has_value() && deck != nullptr,
           "Issue 1704 registered negative-control scenario is unavailable.");
    std::mt19937_64 rng{1};
    sim::TraceLog trace{true, {}};
    sim::Engine engine(*scenario, deck->recipe, rng, &trace);
    static_cast<void>(engine.run());
    expect(!trace_contains(trace, "Issue-1704") &&
               !trace_contains(trace, "scheduled Item-lock route"),
           "The route override must remain limited to T2 Item lock going second.");
  }

  // Retain the old permanent-lock boundary only as an explicit synthetic fixture.
  // Full-turn-one Item-lock labels are intentionally absent from registration:
  // https://assets.pokemon.com/assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/2247
  const sim::Scenario full_lock{"issue-1704-synthetic-full-item-lock",
                                sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, false, 5};
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(deck != nullptr, "Issue 1704 synthetic lock-control deck is unavailable.");
  std::mt19937_64 rng{1};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(full_lock, deck->recipe, rng, &trace);
  static_cast<void>(engine.run());
  expect(!trace_contains(trace, "Issue-1704") &&
             !trace_contains(trace, "scheduled Item-lock route"),
         "The scheduled-lock override must not run under synthetic FullItem.");
}

}  // namespace

int main() {
  try {
    test_registered_seed_reaches_t2_through_burnet();
    test_entry_requires_realistic_vessel_cost();
    test_entry_requires_held_vstar();
    test_other_lock_profiles_keep_existing_policy();
    std::cout << "Issue 1704 Item-lock Burnet route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

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
  static std::optional<Card> treasure_cost(const Engine& engine) {
    return engine.choose_discard_issue1876(
        false, true, true, Card::MysteriousTreasure);
  }
  static void set_prizes_revealed(Engine& engine, const bool value) {
    engine.prizes_revealed_ = value;
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

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.hand = {sim::Card::Dragapult, sim::Card::Klara,
                sim::Card::Crispin, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::TapuLeleGX, sim::Card::RegidragoV};
  return state;
}

sim::Scenario flex(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1876", sim::DciProfile::MatchupFlexJit,
                       lock, true, 5};
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_exact_cost_and_boundaries() {
  std::mt19937_64 rng(1876);
  const sim::Scenario flex_scenario = flex();
  sim::Engine engine = make_engine(flex_scenario, rng, route_state());

  // The visible Dragon supplies the same-turn Apex Dragon payload while held
  // Crispin attaches the missing Grass. Klara retains its recovery role.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1876
  expect(sim::EngineTestAccess::treasure_cost(engine) == sim::Card::Dragapult,
         "The exact K1 GF state did not select the Dragon payload.");

  sim::State no_crispin = route_state();
  no_crispin.hand.erase(std::find(no_crispin.hand.begin(), no_crispin.hand.end(),
                                  sim::Card::Crispin));
  sim::Engine missing = make_engine(flex_scenario, rng, std::move(no_crispin));
  expect(sim::EngineTestAccess::treasure_cost(missing) != sim::Card::Dragapult,
         "Missing Crispin still admitted the payload cost.");

  const sim::Scenario strict_scenario{
      "strict", sim::DciProfile::StrictJit, sim::LockMode::None, true, 5};
  sim::Engine strict = make_engine(strict_scenario, rng, route_state());
  expect(sim::EngineTestAccess::treasure_cost(strict) == sim::Card::Dragapult,
         "Strict JIT did not admit the same current-turn payload route.");

  const sim::Scenario locked_scenario = flex(sim::LockMode::FullItem);
  sim::Engine locked = make_engine(locked_scenario, rng, route_state());
  expect(sim::EngineTestAccess::treasure_cost(locked) != sim::Card::Dragapult,
         "Item lock admitted the Treasure-specific cost.");

  sim::Engine k0 = make_engine(flex_scenario, rng, route_state(), false);
  expect(sim::EngineTestAccess::treasure_cost(k0) != sim::Card::Dragapult,
         "K0 admitted the K1-only cost override.");
}


void test_old_payload_and_prize_inspection_regression() {
  std::mt19937_64 rng(1896);
  // Engine retains this Scenario reference: https://eel.is/c++draft/class.temporary#6.10
  const sim::Scenario flex_scenario = flex();
  sim::State state = route_state();
  state.discard = {sim::Card::DialgaGX};
  sim::Engine engine = make_engine(flex_scenario, rng, state, false);
  sim::EngineTestAccess::set_prizes_revealed(engine, true);

  // A Dialga-GX discarded on an earlier turn does not satisfy current-turn JIT.
  // Legal Prize inspection establishes the fixed-list K1 state, so Treasure may
  // spend Dragapult ex now and held Crispin may attach the final Grass.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Current-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1896
  expect(sim::EngineTestAccess::treasure_cost(engine) == sim::Card::Dragapult,
         "An older payload suppressed the current-turn Treasure payload cost.");

  state.discarded_this_turn = {sim::Card::DialgaGX};
  sim::Engine already_ready =
      make_engine(flex_scenario, rng, std::move(state), false);
  sim::EngineTestAccess::set_prizes_revealed(already_ready, true);
  expect(sim::EngineTestAccess::treasure_cost(already_ready) != sim::Card::Dragapult,
         "An already-satisfied current-turn payload spent another Dragon.");
}

void test_registered_seed_925_reaches_t3_in_both_jit_profiles() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto flex_scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const auto strict_scenario = sim::scenario_by_label("strict-jit/go-second");
  expect(deck != nullptr && flex_scenario.has_value() && strict_scenario.has_value(),
         "The registered issue-1896 fixtures are unavailable.");

  std::mt19937_64 flex_rng(925);
  sim::Engine flex_engine(*flex_scenario, deck->recipe, flex_rng);
  expect(flex_engine.run().first_ready_turn == 3,
         "Matchup-flex seed 925 did not reach readiness on T3.");

  std::mt19937_64 strict_rng(925);
  sim::Engine strict_engine(*strict_scenario, deck->recipe, strict_rng);
  expect(strict_engine.run().first_ready_turn == 3,
         "Strict-JIT seed 925 did not reach readiness on T3.");
}

void test_registered_seed_444_reaches_t3() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1876 fixture is unavailable.");

  std::mt19937_64 rng(444);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Seed 444 did not reach matchup-flex readiness on T3.");
  expect(trace_contains(trace, "Dragapult ex (Mysterious Treasure cost)") ||
             trace_contains(trace, "Dialga-GX (Mysterious Treasure cost)"),
         "Seed 444 did not spend a visible Dragon payload.");
  expect(!trace_contains(trace, "Klara (Mysterious Treasure cost)"),
         "Seed 444 still discarded Klara.");
}
}  // namespace

int main() {
  try {
    test_exact_cost_and_boundaries();
    test_old_payload_and_prize_inspection_regression();
    test_registered_seed_925_reaches_t3_in_both_jit_profiles();
    test_registered_seed_444_reaches_t3();
    std::cout << "Issue 1876 Treasure payload cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

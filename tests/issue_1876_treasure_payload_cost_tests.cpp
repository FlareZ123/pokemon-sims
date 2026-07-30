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
  sim::Engine engine = make_engine(flex(), rng, route_state());

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
  sim::Engine missing = make_engine(flex(), rng, std::move(no_crispin));
  expect(sim::EngineTestAccess::treasure_cost(missing) != sim::Card::Dragapult,
         "Missing Crispin still admitted the payload cost.");

  sim::Engine strict = make_engine(
      sim::Scenario{"strict", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 5},
      rng, route_state());
  expect(sim::EngineTestAccess::treasure_cost(strict) != sim::Card::Dragapult,
         "Strict JIT admitted the matchup-flex cost.");

  sim::Engine locked = make_engine(flex(sim::LockMode::FullItem), rng,
                                   route_state());
  expect(sim::EngineTestAccess::treasure_cost(locked) != sim::Card::Dragapult,
         "Item lock admitted the Treasure-specific cost.");

  sim::Engine k0 = make_engine(flex(), rng, route_state(), false);
  expect(sim::EngineTestAccess::treasure_cost(k0) != sim::Card::Dragapult,
         "K0 admitted the K1-only cost override.");
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
    test_registered_seed_444_reaches_t3();
    std::cout << "Issue 1876 Treasure payload cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

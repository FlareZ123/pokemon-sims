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
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
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

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_no_lock_admits_proven_package_only() {
  const sim::Scenario no_lock_scenario{"issue-1378-no-lock",
                                       sim::DciProfile::StrictJit,
                                       sim::LockMode::None, false, 5};
  const sim::Scenario scheduled_lock_scenario{"issue-1378-scheduled-lock",
                                               sim::DciProfile::StrictJit,
                                               sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{137801};
  sim::Engine no_lock = make_engine(no_lock_scenario, rng, route_state());

  // The held VSTAR already completes the Evolution axis. Steven's third target
  // must be Grass so the manual attachment plus Crispin establish GGF:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official search, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original guarded package: https://github.com/FlareZ123/pokemon-sims/issues/1223
  // Confirmed no-lock bug: https://github.com/FlareZ123/pokemon-sims/issues/1378
  expect(sim::EngineTestAccess::route_available(no_lock),
         "The proven Steven Grass package was unavailable without locks.");

  sim::State no_vstar = route_state();
  const auto vstar = std::find(no_vstar.hand.begin(), no_vstar.hand.end(),
                               sim::Card::RegidragoVstar);
  no_vstar.hand.erase(vstar);
  sim::Engine direct_vstar = make_engine(no_lock_scenario, rng,
                                         std::move(no_vstar));
  expect(!sim::EngineTestAccess::route_available(direct_vstar),
         "A missing held VSTAR failed to preserve direct VSTAR search.");

  sim::Engine scheduled_lock = make_engine(scheduled_lock_scenario, rng,
                                            route_state());
  expect(!sim::EngineTestAccess::route_available(scheduled_lock),
         "The no-lock generalization leaked into a scheduled-lock policy.");
}

void test_exact_seed_12_improves_to_t3_with_latias_follow_up() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered no-lock seed-12 fixture is unavailable.");

  std::mt19937_64 rng{12};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "The corrected no-lock seed 12 did not reach the complete T3 route.");
  expect(trace_contains(
             trace,
             "Searched up to 3 cards: Regidrago V, Crispin, Grass Energy"),
         "Steven still selected the redundant held Regidrago VSTAR axis.");
  expect(trace_contains(trace, "Searched any card: Latias ex"),
         "Star Alchemy did not resolve the remaining Active-position axis.");
  // Expected route marker: STAR ALCHEMY searched Latias ex.
  // The independently merged Steven correction supplies the GG foundation.
  // Issue #1379 completes the same public continuation by searching Latias ex,
  // attaching Fire, evolving, using Burnet, and retreating the Basic Active:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed dependent improvement: https://github.com/FlareZ123/pokemon-sims/issues/1379
}

}  // namespace

int main() {
  try {
    test_no_lock_admits_proven_package_only();
    test_exact_seed_12_improves_to_t3_with_latias_follow_up();
    std::cout << "Issue 1378 no-lock Steven Grass tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

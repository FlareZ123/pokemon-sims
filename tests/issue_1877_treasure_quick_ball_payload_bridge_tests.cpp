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
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1877_treasure_quick_ball_payload_bridge_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1877_treasure_quick_ball_payload_bridge();
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

sim::Scenario strict(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1877", sim::DciProfile::StrictJit, lock, false, 5};
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                sim::Card::QuickBall, sim::Card::FieldBlower};
  state.deck = {sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::RegidragoV, sim::Card::LatiasEx};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                   prizes_revealed);
  return engine;
}

void test_exact_route_and_knowledge_boundaries() {
  std::mt19937_64 rng(1877);
  const sim::Scenario scenario = strict();
  sim::Engine deck_search_k1 = make_engine(scenario, rng, route_state());

  // Earthen Vessel is route-replaced after GGF is complete. Treasure converts it
  // into a searched Dragon, and Quick Ball converts that Dragon into the same-turn
  // payload while completing a legal Basic search.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1877
  expect(sim::EngineTestAccess::route_available(deck_search_k1),
         "Deck-search K1 did not admit the Treasure-to-Quick-Ball bridge.");
  expect(sim::EngineTestAccess::complete_route(deck_search_k1),
         "The exact Treasure-to-Quick-Ball bridge did not complete.");
  expect(deck_search_k1.state().active->grass == 2 &&
             deck_search_k1.state().active->fire == 1,
         "The bridge disturbed the complete GGF axis.");
  expect(std::any_of(deck_search_k1.state().discarded_this_turn.begin(),
                     deck_search_k1.state().discarded_this_turn.end(),
                     sim::is_payload),
         "Quick Ball did not discard the searched Dragon payload.");

  // Full Prize inspection also establishes K1 by revealing the complementary deck.
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  sim::Engine prize_k1 = make_engine(scenario, rng, route_state(), false, true);
  expect(sim::EngineTestAccess::route_available(prize_k1),
         "Prize-inspection K1 did not admit the public route.");
  expect(sim::EngineTestAccess::complete_route(prize_k1),
         "Prize-inspection K1 did not complete the public route.");

  sim::Engine k0 = make_engine(scenario, rng, route_state(), false, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "True K0 admitted the K1-only bridge.");
}

void test_legality_and_resource_boundaries() {
  std::mt19937_64 rng(18770);
  const sim::Scenario scenario = strict();

  // Quick Ball only searches a Basic into hand, so a full Bench does not block
  // this exact bridge.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Official Bench and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  sim::State full_bench = route_state();
  full_bench.bench.assign(5, sim::Pokemon{sim::Card::RegidragoV, 1});
  sim::Engine benched = make_engine(scenario, rng, std::move(full_bench));
  expect(sim::EngineTestAccess::route_available(benched),
         "A full Bench incorrectly blocked a Basic search that may remain in hand.");

  const sim::Scenario item_lock_scenario = strict(sim::LockMode::FullItem);
  sim::Engine item_locked = make_engine(item_lock_scenario, rng, route_state());
  expect(!sim::EngineTestAccess::route_available(item_locked),
         "Item lock admitted the two-Item bridge.");

  sim::State missing_vessel = route_state();
  missing_vessel.hand.erase(std::find(missing_vessel.hand.begin(),
                                      missing_vessel.hand.end(),
                                      sim::Card::EarthenVessel));
  sim::Engine no_vessel = make_engine(scenario, rng, std::move(missing_vessel));
  expect(!sim::EngineTestAccess::route_available(no_vessel),
         "The route was admitted without its explicit Treasure cost.");

  sim::State missing_payload = route_state();
  missing_payload.deck.erase(
      std::remove_if(missing_payload.deck.begin(), missing_payload.deck.end(),
                     sim::is_payload),
      missing_payload.deck.end());
  sim::Engine no_payload = make_engine(scenario, rng, std::move(missing_payload));
  expect(!sim::EngineTestAccess::route_available(no_payload),
         "The route was admitted without a searchable Dragon payload.");

  sim::State missing_basic = route_state();
  missing_basic.deck = {sim::Card::Dragapult, sim::Card::MegaDragonite};
  sim::Engine no_basic = make_engine(scenario, rng, std::move(missing_basic));
  expect(!sim::EngineTestAccess::route_available(no_basic),
         "The route was admitted without a legal Quick Ball target.");

  sim::State already_payload = route_state();
  already_payload.discarded_this_turn.push_back(sim::Card::Dragapult);
  sim::Engine complete = make_engine(scenario, rng, std::move(already_payload));
  expect(!sim::EngineTestAccess::route_available(complete),
         "An already-complete payload axis admitted the redundant bridge.");
}

void test_registered_seed_740_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1877 fixture is unavailable.");

  // Seed 169 now follows the strictly faster T2 Quick Ball, Tapu Lele-GX, and
  // Professor Burnet route fixed by issue 1869, so seed 740 is the current
  // source-bound witness for the original Treasure-to-Quick-Ball bridge:
  // https://github.com/FlareZ123/pokemon-sims/issues/1869
  // https://github.com/FlareZ123/pokemon-sims/issues/1877
  std::mt19937_64 rng(740);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  expect(outcome.first_ready_turn == 3,
         "Seed 740 did not reach strict-JIT readiness on T3.");
  expect(trace_contains(trace,
                        "Mysterious Treasure spent route-replaced Earthen Vessel") &&
             trace_contains(trace,
                            "Quick Ball discarded the searched Dragon payload"),
         "Seed 740 did not use the corrected Treasure-to-Quick-Ball bridge.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_knowledge_boundaries();
    test_legality_and_resource_boundaries();
    test_registered_seed_740_reaches_t3();
    std::cout << "Issue 1877 Treasure-to-Quick-Ball bridge tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

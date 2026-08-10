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
    return engine.issue_1871_treasure_serena_payload_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1871_treasure_serena_payload_route();
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

sim::Scenario jit_first(const sim::DciProfile profile,
                        const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1871", profile, lock, true, 5};
}

sim::Scenario strict_first(const sim::LockMode lock = sim::LockMode::None) {
  return jit_first(sim::DciProfile::StrictJit, lock);
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Serena,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV,
                sim::Card::Fire, sim::Card::Grass, sim::Card::Channeler};
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

void test_exact_route_and_k1_boundaries() {
  std::mt19937_64 rng(1871);
  const sim::Scenario scenario = strict_first();
  sim::Engine engine = make_engine(scenario, rng, base_state());
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact issue-1871 route was unavailable.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact issue-1871 route did not complete.");
  expect(engine.state().active->grass == 2 && engine.state().active->fire == 1,
         "The issue-1871 route did not complete GGF.");
  expect(std::find(engine.state().discarded_this_turn.begin(),
                   engine.state().discarded_this_turn.end(),
                   sim::Card::MegaDragonite) !=
             engine.state().discarded_this_turn.end(),
         "Serena did not discard the searched Dragon this turn.");
  expect(std::find(engine.state().hand.begin(), engine.state().hand.end(),
                   sim::Card::BrilliantBlender) != engine.state().hand.end(),
         "The deterministic route replayed Brilliant Blender.");

  sim::Engine prize_k1 = make_engine(scenario, rng, base_state(), false, true);
  expect(sim::EngineTestAccess::route_available(prize_k1),
         "Full Prize inspection K1 did not admit the route.");
  sim::Engine k0 = make_engine(scenario, rng, base_state(), false, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the deterministic payload search route.");
}

void test_jit_profile_parity() {
  std::mt19937_64 rng(2741);
  sim::Engine strict = make_engine(
      jit_first(sim::DciProfile::StrictJit), rng, base_state());
  sim::Engine flexible = make_engine(
      jit_first(sim::DciProfile::MatchupFlexJit), rng, base_state());
  // Both JIT profiles require the Dragon payload on the ready turn: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Mysterious Treasure, Serena, and Apex Dragon: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12-164 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed profile-overfitting bug: https://github.com/FlareZ123/pokemon-sims/issues/2741
  expect(sim::EngineTestAccess::route_available(strict),
         "Strict JIT lost the issue-1871 route.");
  expect(sim::EngineTestAccess::route_available(flexible),
         "Matchup-flex JIT did not admit the same legal issue-1871 route.");
}

void test_lock_and_resource_boundaries() {
  std::mt19937_64 rng(292);
  const sim::Scenario no_lock = strict_first();
  const sim::Scenario item_lock = strict_first(sim::LockMode::FullItem);
  const sim::Scenario supporter_lock = strict_first(sim::LockMode::FullSupporter);

  sim::Engine items = make_engine(item_lock, rng, base_state());
  expect(!sim::EngineTestAccess::route_available(items),
         "Item lock admitted Mysterious Treasure.");
  sim::Engine supporters = make_engine(supporter_lock, rng, base_state());
  expect(!sim::EngineTestAccess::route_available(supporters),
         "Supporter lock admitted Serena.");

  sim::State one_grass = base_state();
  one_grass.hand.erase(std::find(one_grass.hand.begin(), one_grass.hand.end(),
                                 sim::Card::Grass));
  sim::Engine missing_cost = make_engine(no_lock, rng, std::move(one_grass));
  expect(!sim::EngineTestAccess::route_available(missing_cost),
         "The route was admitted without both attachment and Item-cost Grass.");

  sim::State no_payload = base_state();
  no_payload.deck.erase(std::find(no_payload.deck.begin(), no_payload.deck.end(),
                                  sim::Card::MegaDragonite));
  sim::Engine missing_payload = make_engine(no_lock, rng, std::move(no_payload));
  expect(!sim::EngineTestAccess::route_available(missing_payload),
         "The route was admitted without a searchable Dragon payload.");
}

void test_registered_seed_292_reaches_t3_for_both_jit_profiles() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered issue-1871 deck is unavailable.");

  for (const char* label : {"strict-jit/go-first", "matchup-flex-jit/go-first"}) {
    const auto scenario = sim::scenario_by_label(label);
    expect(scenario.has_value(), "A registered issue-1871 scenario is unavailable.");
    std::mt19937_64 rng(292);
    sim::TraceLog trace{true, {}, {}};
    sim::Engine engine(*scenario, deck->recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();
    // Earliest-route policy requires the same legal deterministic finish in either JIT profile: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed profile-overfitting bug: https://github.com/FlareZ123/pokemon-sims/issues/2741
    expect(outcome.first_ready_turn == 3,
           "Registered seed 292 did not reach T3 in both JIT profiles.");
    expect(trace_contains(trace, "Mysterious Treasure") &&
               trace_contains(trace, "Serena") &&
               trace_contains(trace, "T3 | READY"),
           "Registered seed 292 did not execute the complete T3 route.");
  }
}
}  // namespace

int main() {
  try {
    test_exact_route_and_k1_boundaries();
    test_jit_profile_parity();
    test_lock_and_resource_boundaries();
    test_registered_seed_292_reaches_t3_for_both_jit_profiles();
    std::cout << "Issue 1871 Treasure-Serena payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
